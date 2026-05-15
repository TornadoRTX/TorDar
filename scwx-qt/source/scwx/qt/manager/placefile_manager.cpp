#include <scwx/qt/manager/placefile_manager.hpp>
#include <scwx/qt/manager/font_manager.hpp>
#include <scwx/qt/manager/resource_manager.hpp>
#include <scwx/qt/main/application.hpp>
#include <scwx/qt/main/application_paths.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/util/network.hpp>
#include <scwx/gr/placefile.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/util/json.hpp>
#include <scwx/util/logger.hpp>

#include <atomic>
#include <array>
#include <shared_mutex>
#include <type_traits>
#include <vector>

#include <QDir>
#include <QGuiApplication>
#include <QScreen>
#include <QUrl>
#include <boost/algorithm/string.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/json.hpp>
#include <boost/tokenizer.hpp>
#include <cpr/cpr.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#if SCWX_HAS_NETCDF
#   include <netcdf.h>
#endif

#include <fstream>
#include <iomanip>
#include <regex>
#include <algorithm>
#include <filesystem>

namespace scwx::qt::manager
{

static const std::string logPrefix_ = "scwx::qt::manager::placefile_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static const std::string kEnabledName_          = "enabled";
static const std::string kThresholdedName_      = "thresholded";
static const std::string kTitleName_            = "title";
static const std::string kNameName_             = "name";
static const std::string kLegacyLightningTitle_ = "Legacy Lightning";
static const std::string kLegacyLightningUrl_ =
   "https://www.freelightning.com/hub/"
   "placefile.php?request=10213|10454|138624046|10463|10369|10644|0|84764|1";
static const std::string kGoesGlmLightningTitle_ = "GOES GLM Lightning (AWS)";
static const std::string kGoesGlmLightningUrl_ =
   "https://noaa-goes16.s3.amazonaws.com/index.html";
static const std::string kGoesGlmBucketUrl_ =
   "https://noaa-goes16.s3.amazonaws.com";
static constexpr std::chrono::minutes kGoesGlmRetention_ {15};

struct GlmLightningPoint
{
   double                                latitude_ {};
   double                                longitude_ {};
   std::chrono::system_clock::time_point time_ {};
};

static std::chrono::system_clock::time_point UtcNowMinute()
{
   const auto now = std::chrono::system_clock::now();
   return std::chrono::time_point_cast<std::chrono::minutes>(now);
}

static std::chrono::system_clock::time_point UtcTmToTimePoint(std::tm tm)
{
#if defined(_WIN32)
   const auto epoch = _mkgmtime(&tm);
#else
   const auto epoch = timegm(&tm);
#endif
   return std::chrono::system_clock::from_time_t(epoch);
}

static bool ParseGlmFileStartTime(const std::string& filename,
                                  std::chrono::system_clock::time_point* t)
{
   static const std::regex kStartTimeRegex {
      R"(_s(\d{4})(\d{3})(\d{2})(\d{2})(\d{2}))"};
   std::smatch match {};
   if (!std::regex_search(filename, match, kStartTimeRegex))
   {
      return false;
   }

   std::tm tm {};
   tm.tm_year = std::stoi(match[1].str()) - 1900;
   tm.tm_mon  = 0;
   tm.tm_mday = 1;
   tm.tm_hour = std::stoi(match[3].str());
   tm.tm_min  = std::stoi(match[4].str());
   tm.tm_sec  = std::stoi(match[5].str());

   const auto jan1         = UtcTmToTimePoint(tm);
   const int  dayOfYearOne = std::stoi(match[2].str());
   *t                      = jan1 + std::chrono::hours(24 * (dayOfYearOne - 1));
   return true;
}

static std::vector<std::string> ParseS3Keys(const std::string& xmlBody)
{
   std::vector<std::string> keys {};
   static const std::regex  kKeyRegex {R"(<Key>([^<]+)</Key>)"};

   auto begin = std::sregex_iterator(xmlBody.begin(), xmlBody.end(), kKeyRegex);
   auto end   = std::sregex_iterator();

   for (auto it = begin; it != end; ++it)
   {
      keys.push_back((*it)[1].str());
   }

   return keys;
}

static std::vector<std::string>
FetchRecentGlmKeys(const std::chrono::system_clock::time_point& nowUtc)
{
   std::vector<std::string> keys {};

   for (int hourOffset = 0; hourOffset >= -1; --hourOffset)
   {
      const auto hourTime = nowUtc + std::chrono::hours(hourOffset);
      const auto tt       = std::chrono::system_clock::to_time_t(hourTime);
      std::tm    tmUtc {};
#if defined(_WIN32)
      gmtime_s(&tmUtc, &tt);
#else
      gmtime_r(&tt, &tmUtc);
#endif

      const std::string prefix =
         fmt::format("GLM-L2-LCFA/{:04d}/{:03d}/{:02d}/",
                     tmUtc.tm_year + 1900,
                     tmUtc.tm_yday + 1,
                     tmUtc.tm_hour);

      auto response = cpr::Get(cpr::Url {kGoesGlmBucketUrl_},
                               network::cpr::GetHeader(),
                               cpr::Parameters {{"list-type", "2"},
                                                {"max-keys", "1000"},
                                                {"prefix", prefix}},
                               network::cpr::GetDefaultTimeout(),
                               network::cpr::GetDefaultConnectTimeout(),
                               network::cpr::GetDefaultLowSpeed());

      if (!cpr::status::is_success(response.status_code))
      {
         continue;
      }

      auto parsed = ParseS3Keys(response.text);
      keys.insert(keys.end(), parsed.begin(), parsed.end());
   }

   return keys;
}

#if SCWX_HAS_NETCDF
template<typename T>
static bool ReadNetcdfVariable(int ncid, const char* name, std::vector<T>* out)
{
   int varid {};
   if (nc_inq_varid(ncid, name, &varid) != NC_NOERR)
   {
      return false;
   }

   int ndims {};
   if (nc_inq_varndims(ncid, varid, &ndims) != NC_NOERR || ndims < 1)
   {
      return false;
   }

   int dimid {};
   if (nc_inq_vardimid(ncid, varid, &dimid) != NC_NOERR)
   {
      return false;
   }

   size_t count {};
   if (nc_inq_dimlen(ncid, dimid, &count) != NC_NOERR || count == 0)
   {
      return false;
   }

   out->resize(count);
   if constexpr (std::is_same_v<T, float>)
   {
      return nc_get_var_float(ncid, varid, out->data()) == NC_NOERR;
   }
   else
   {
      return nc_get_var_double(ncid, varid, out->data()) == NC_NOERR;
   }
}

static std::chrono::system_clock::time_point ReadCoverageStart(int ncid)
{
   std::array<char, 64> buffer {};
   if (nc_get_att_text(ncid, NC_GLOBAL, "time_coverage_start", buffer.data()) ==
       NC_NOERR)
   {
      std::tm            tm {};
      std::istringstream ss {std::string {buffer.data()}};
      ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
      if (!ss.fail())
      {
         return UtcTmToTimePoint(tm);
      }
   }

   return UtcNowMinute();
}
#endif

static std::vector<GlmLightningPoint>
BuildGlmPointsFromFile(const std::filesystem::path& ncFile)
{
   std::vector<GlmLightningPoint> points {};

#if SCWX_HAS_NETCDF
   int ncid {};
   if (nc_open(ncFile.string().c_str(), NC_NOWRITE, &ncid) != NC_NOERR)
   {
      return points;
   }

   std::vector<float> latitudes {};
   std::vector<float> longitudes {};

   const bool hasFlashCoords =
      ReadNetcdfVariable(ncid, "flash_lat", &latitudes) &&
      ReadNetcdfVariable(ncid, "flash_lon", &longitudes);
   const bool hasGroupCoords =
      !hasFlashCoords && ReadNetcdfVariable(ncid, "group_lat", &latitudes) &&
      ReadNetcdfVariable(ncid, "group_lon", &longitudes);

   std::vector<double> offsets {};
   if (hasFlashCoords)
   {
      ReadNetcdfVariable(ncid, "flash_time_offset_of_first_event", &offsets);
   }
   else if (hasGroupCoords)
   {
      ReadNetcdfVariable(ncid, "group_time_offset", &offsets);
   }

   const auto baseTime = ReadCoverageStart(ncid);
   nc_close(ncid);

   const size_t count =
      std::min(latitudes.size(), std::min(longitudes.size(), offsets.size()));
   points.reserve(count);

   for (size_t i = 0; i < count; ++i)
   {
      points.push_back(
         {latitudes[i],
          longitudes[i],
          baseTime + std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::duration<double>(offsets[i]))});
   }
#else
   (void) ncFile;
#endif

   return points;
}

static std::shared_ptr<gr::Placefile>
BuildGoesGlmPlacefile(const std::string& placefileName)
{
   const auto nowUtc = UtcNowMinute();
   const auto cutoff = nowUtc - kGoesGlmRetention_;

   std::vector<std::pair<std::string, std::chrono::system_clock::time_point>>
      candidates {};

   for (const auto& key : FetchRecentGlmKeys(nowUtc))
   {
      std::chrono::system_clock::time_point startTime {};
      if (key.ends_with(".nc") && ParseGlmFileStartTime(key, &startTime) &&
          startTime >= cutoff)
      {
         candidates.emplace_back(key, startTime);
      }
   }

   std::sort(candidates.begin(),
             candidates.end(),
             [](const auto& lhs, const auto& rhs)
             { return lhs.second > rhs.second; });

   if (candidates.size() > 45)
   {
      candidates.resize(45);
   }

   std::vector<GlmLightningPoint> points {};
   points.reserve(5000);

   for (const auto& [key, fileStart] : candidates)
   {
      (void) fileStart;
      const std::string url = fmt::format("{}/{}", kGoesGlmBucketUrl_, key);
      auto              response = cpr::Get(cpr::Url {url},
                               network::cpr::GetHeader(),
                               network::cpr::GetDefaultTimeout(),
                               network::cpr::GetDefaultConnectTimeout(),
                               network::cpr::GetDefaultLowSpeed());
      if (!cpr::status::is_success(response.status_code))
      {
         continue;
      }

      auto tempPath =
         std::filesystem::temp_directory_path() /
         fmt::format("scwx_glm_{}.nc",
                     std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count());

      {
         std::ofstream ofs {tempPath, std::ios::binary | std::ios::trunc};
         ofs.write(response.text.data(),
                   static_cast<std::streamsize>(response.text.size()));
      }

      auto            filePoints = BuildGlmPointsFromFile(tempPath);
      std::error_code ec {};
      std::filesystem::remove(tempPath, ec);

      points.insert(points.end(), filePoints.begin(), filePoints.end());
   }

   std::ostringstream pf {};
   pf << "Title: " << kGoesGlmLightningTitle_ << "\n";
   pf << "RefreshSeconds: 30\n";
   pf << "Threshold: 999\n";
   pf << "IconFile: 1, 28, 28, 14, 14, "
         "\":/res/icons/flaticon/lightning.svg\"\n";

   for (const auto& p : points)
   {
      if (p.time_ < cutoff || p.time_ > nowUtc)
      {
         continue;
      }

      const auto endTime = p.time_ + std::chrono::minutes(16);

      const auto startT = std::chrono::system_clock::to_time_t(p.time_);
      const auto endT   = std::chrono::system_clock::to_time_t(endTime);
      std::tm    startTm {};
      std::tm    endTm {};
#if defined(_WIN32)
      gmtime_s(&startTm, &startT);
      gmtime_s(&endTm, &endT);
#else
      gmtime_r(&startT, &startTm);
      gmtime_r(&endT, &endTm);
#endif

      pf << "TimeRange: " << fmt::format("{:%Y-%m-%dT%H:%M:%S}", startTm) << " "
         << fmt::format("{:%Y-%m-%dT%H:%M:%S}", endTm) << "\n";
      pf << "Icon: "
         << fmt::format("{:.4f}, {:.4f}, 0, 1, 1", p.latitude_, p.longitude_)
         << "\n";
   }

   std::istringstream stream {pf.str()};
   return gr::Placefile::Load(placefileName, stream);
}

static bool ContainsPlacefileEntry(const boost::json::value& placefileJson,
                                   const std::string&        name)
{
   if (!placefileJson.is_array())
   {
      return false;
   }

   for (const auto& entry : placefileJson.as_array())
   {
      if (!entry.is_object())
      {
         continue;
      }

      auto it = entry.as_object().find(kNameName_);
      if (it != entry.as_object().end() && it->value().is_string() &&
          it->value().as_string() == name)
      {
         return true;
      }
   }

   return false;
}

class PlacefileManager::Impl
{
public:
   class PlacefileRecord;

   explicit Impl(PlacefileManager* self) : self_ {self} {}
   ~Impl() { threadPool_.join(); }

   void InitializePlacefileSettings();
   void SyncBuiltInLightning();
   void ApplyPlacefileSettings(const boost::json::value& placefileJson);
   void ReadPlacefileSettings();
   void SavePlacefileSettings();

   static FontMap
   LoadFontResources(const std::shared_ptr<gr::Placefile>& placefile);
   static std::vector<std::shared_ptr<boost::gil::rgba8_image_t>>
   LoadImageResources(const std::shared_ptr<gr::Placefile>& placefile);

   boost::asio::thread_pool threadPool_ {1u};

   PlacefileManager* self_;

   std::string placefileSettingsPath_ {};

   std::shared_ptr<config::RadarSite> radarSite_ {};

   std::vector<std::shared_ptr<PlacefileRecord>> placefileRecords_ {};
   boost::unordered_flat_map<std::string, std::shared_ptr<PlacefileRecord>>
                     placefileRecordMap_ {};
   std::shared_mutex placefileRecordLock_ {};

   bool placefileSettingsRead_ {false};
};

class PlacefileManager::Impl::PlacefileRecord
{
public:
   explicit PlacefileRecord(Impl*                          impl,
                            const std::string&             name,
                            std::shared_ptr<gr::Placefile> placefile,
                            const std::string&             title   = {},
                            bool                           enabled = false,
                            bool thresholded                       = false) :
       p {impl},
       name_ {name},
       title_ {title},
       placefile_ {placefile},
       enabled_ {enabled},
       thresholded_ {thresholded}
   {
   }
   ~PlacefileRecord()
   {
      std::unique_lock refreshLock(refreshMutex_);
      std::unique_lock timerLock(timerMutex_);
      enabled_ = false;
      refreshTimer_.cancel();
      timerLock.unlock();
      refreshLock.unlock();

      threadPool_.join();
   }

   bool                 refresh_enabled() const;
   std::chrono::seconds refresh_time() const;

   void CancelRefresh();
   void ScheduleRefresh();
   void ScheduleRefresh(
      const std::chrono::system_clock::duration timeUntilNextUpdate);
   void Update();
   void UpdateAsync();

   friend void tag_invoke(boost::json::value_from_tag,
                          boost::json::value&                     jv,
                          const std::shared_ptr<PlacefileRecord>& record)
   {
      jv = {{kEnabledName_, record->enabled_.load()},
            {kThresholdedName_, record->thresholded_},
            {kTitleName_, record->title_},
            {kNameName_, record->name_}};
   }

   friend PlacefileRecord tag_invoke(boost::json::value_to_tag<PlacefileRecord>,
                                     const boost::json::value& jv)
   {
      return PlacefileRecord {
         nullptr,
         boost::json::value_to<std::string>(jv.at(kNameName_)),
         nullptr,
         boost::json::value_to<std::string>(jv.at(kTitleName_)),
         jv.at(kEnabledName_).as_bool(),
         jv.at(kThresholdedName_).as_bool()};
   }

   Impl* p;

   std::string                    name_;
   std::string                    title_;
   std::shared_ptr<gr::Placefile> placefile_;
   std::atomic<bool>              enabled_;
   bool                           thresholded_;
   boost::asio::thread_pool       threadPool_ {1u};
   boost::asio::steady_timer      refreshTimer_ {threadPool_};
   std::mutex                     refreshMutex_ {};
   std::mutex                     timerMutex_ {};

   FontMap    fonts_ {};
   std::mutex fontsMutex_ {};

   std::vector<std::shared_ptr<boost::gil::rgba8_image_t>> images_ {};

   std::string                           lastRadarSite_ {};
   std::chrono::system_clock::time_point lastUpdateTime_ {};

   std::size_t failureCount_ {};
};

PlacefileManager::PlacefileManager() : p(std::make_unique<Impl>(this))
{
   settings::GeneralSettings::Instance()
      .legacy_lightning_enabled()
      .RegisterValueChangedCallback([this](const bool& /* enabled */)
                                    { p->SyncBuiltInLightning(); });
   settings::GeneralSettings::Instance()
      .goes_glm_lightning_enabled()
      .RegisterValueChangedCallback([this](const bool& /* enabled */)
                                    { p->SyncBuiltInLightning(); });

   boost::asio::post(p->threadPool_,
                     [this]()
                     {
                        try
                        {
                           p->InitializePlacefileSettings();

                           // Read placefile settings on startup
                           main::Application::WaitForInitialization();
                           p->ReadPlacefileSettings();
                           Q_EMIT PlacefilesInitialized();
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

PlacefileManager::~PlacefileManager()
{
   // Save placefile settings on shutdown
   p->SavePlacefileSettings();
};

bool PlacefileManager::placefile_enabled(const std::string& name)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      return it->second->enabled_;
   }
   return false;
}

bool PlacefileManager::placefile_thresholded(const std::string& name)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      return it->second->thresholded_;
   }
   return false;
}

std::string PlacefileManager::placefile_title(const std::string& name)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      return it->second->title_;
   }
   return {};
}

std::shared_ptr<gr::Placefile>
PlacefileManager::placefile(const std::string& name)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      return it->second->placefile_;
   }
   return nullptr;
}

PlacefileManager::FontMap
PlacefileManager::placefile_fonts(const std::string& name)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      std::unique_lock fontsLock {it->second->fontsMutex_};
      return it->second->fonts_;
   }
   return {};
}

void PlacefileManager::set_placefile_enabled(const std::string& name,
                                             bool               enabled)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      auto record      = it->second;
      record->enabled_ = enabled;

      lock.unlock();

      Q_EMIT PlacefileEnabled(name, enabled);

      using namespace std::chrono_literals;

      // Update the placefile
      if (enabled)
      {
         if (p->radarSite_ != nullptr &&
             record->lastRadarSite_ != p->radarSite_->id())
         {
            // If the radar site has changed, update now
            record->UpdateAsync();
         }
         else
         {
            // Otherwise, schedule an update
            record->ScheduleRefresh();
         }
      }
      else if (!enabled)
      {
         record->CancelRefresh();
      }
   }
}

void PlacefileManager::set_placefile_thresholded(const std::string& name,
                                                 bool               thresholded)
{
   std::shared_lock lock(p->placefileRecordLock_);

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      it->second->thresholded_ = thresholded;

      lock.unlock();

      Q_EMIT PlacefileUpdated(name);
   }
}

void PlacefileManager::set_placefile_url(const std::string& name,
                                         const std::string& newUrl)
{
   std::string normalizedUrl = util::network::NormalizeUrl(newUrl);

   std::unique_lock lock(p->placefileRecordLock_);

   auto it    = p->placefileRecordMap_.find(name);
   auto itNew = p->placefileRecordMap_.find(normalizedUrl);
   if (it != p->placefileRecordMap_.cend() &&
       itNew == p->placefileRecordMap_.cend())
   {
      auto placefileRecord        = it->second;
      placefileRecord->name_      = normalizedUrl;
      placefileRecord->placefile_ = nullptr;
      placefileRecord->fonts_.clear();
      placefileRecord->images_.clear();
      p->placefileRecordMap_.erase(it);
      p->placefileRecordMap_.insert_or_assign(normalizedUrl, placefileRecord);

      lock.unlock();

      Q_EMIT PlacefileRenamed(name, normalizedUrl);

      // Queue a placefile update
      placefileRecord->UpdateAsync();
   }
}

bool PlacefileManager::Impl::PlacefileRecord::refresh_enabled() const
{
   if (placefile_ != nullptr)
   {
      using namespace std::chrono_literals;
      return placefile_->refresh() > 0s;
   }

   return false;
}

std::chrono::seconds
PlacefileManager::Impl::PlacefileRecord::refresh_time() const
{
   using namespace std::chrono_literals;

   if (refresh_enabled())
   {
      // Don't refresh more often than every 1 second
      return std::max(placefile_->refresh(), 1s);
   }

   return -1s;
}

void PlacefileManager::Impl::InitializePlacefileSettings()
{
   const std::string settingsPath {
      main::ApplicationPaths::GetLocation(
         main::ApplicationPaths::StandardLocation::Settings)
         .generic_string()};

   if (!std::filesystem::exists(settingsPath))
   {
      logger_->error("Settings path does not exist: \"{}\"", settingsPath);
   }

   placefileSettingsPath_ = settingsPath + "/placefiles.json";
}

void PlacefileManager::Impl::ReadPlacefileSettings()
{
   logger_->info("Reading placefile settings");

   boost::json::value placefileJson = nullptr;

   // Determine if placefile settings exists
   if (std::filesystem::exists(placefileSettingsPath_))
   {
      placefileJson = scwx::util::json::ReadJsonFile(placefileSettingsPath_);
   }

   ApplyPlacefileSettings(placefileJson);

   // Add and enable the default legacy lightning placefile on first run if
   // enabled.
   if (settings::GeneralSettings::Instance()
          .legacy_lightning_enabled()
          .GetValue() &&
       !ContainsPlacefileEntry(placefileJson, kLegacyLightningUrl_))
   {
      self_->AddUrl(kLegacyLightningUrl_, kLegacyLightningTitle_, true, false);
   }

   // Add GOES GLM lightning source on first run if enabled.
   if (settings::GeneralSettings::Instance()
          .goes_glm_lightning_enabled()
          .GetValue() &&
       !ContainsPlacefileEntry(placefileJson, kGoesGlmLightningUrl_))
   {
      self_->AddUrl(
         kGoesGlmLightningUrl_, kGoesGlmLightningTitle_, true, false);
   }

   // Ensure persisted records match the current built-in lightning setting.
   SyncBuiltInLightning();

   placefileSettingsRead_ = true;
}

void PlacefileManager::Impl::SyncBuiltInLightning()
{
   const bool legacyLightningEnabledRaw = settings::GeneralSettings::Instance()
                                             .legacy_lightning_enabled()
                                             .GetValue();
   const bool goesGlmLightningEnabled = settings::GeneralSettings::Instance()
                                           .goes_glm_lightning_enabled()
                                           .GetValue();
   const bool legacyLightningEnabled =
      legacyLightningEnabledRaw && !goesGlmLightningEnabled;

   std::shared_lock lock(placefileRecordLock_);
   const bool       hasLegacyLightningRecord =
      placefileRecordMap_.find(kLegacyLightningUrl_) !=
      placefileRecordMap_.cend();
   const bool hasGoesGlmLightningRecord =
      placefileRecordMap_.find(kGoesGlmLightningUrl_) !=
      placefileRecordMap_.cend();
   lock.unlock();

   if (legacyLightningEnabled && !hasLegacyLightningRecord)
   {
      self_->AddUrl(kLegacyLightningUrl_, kLegacyLightningTitle_, true, false);
   }
   else if (!legacyLightningEnabled && hasLegacyLightningRecord)
   {
      self_->RemoveUrl(kLegacyLightningUrl_);
   }

   if (goesGlmLightningEnabled && !hasGoesGlmLightningRecord)
   {
      self_->AddUrl(
         kGoesGlmLightningUrl_, kGoesGlmLightningTitle_, true, false);
   }
   else if (!goesGlmLightningEnabled && hasGoesGlmLightningRecord)
   {
      self_->RemoveUrl(kGoesGlmLightningUrl_);
   }
}

void PlacefileManager::ReadPlacefileSettings(std::istream& is)
{
   logger_->info("Reading placefile settings from stream");

   const boost::json::value placefileJson =
      scwx::util::json::ReadJsonStream(is);

   p->ApplyPlacefileSettings(placefileJson);

   // Don't set placefileSettingsRead_ when reading from a non-default stream
}

void PlacefileManager::Impl::ApplyPlacefileSettings(
   const boost::json::value& placefileJson)
{
   // If placefile settings was successfully read
   if (placefileJson != nullptr && placefileJson.is_array())
   {
      // For each placefile entry
      auto& placefileArray = placefileJson.as_array();
      for (auto& placefileEntry : placefileArray)
      {
         try
         {
            // Convert placefile entry to a record
            PlacefileRecord record =
               boost::json::value_to<PlacefileRecord>(placefileEntry);

            if (!record.name_.empty())
            {
               self_->AddUrl(record.name_,
                             record.title_,
                             record.enabled_,
                             record.thresholded_);
            }
         }
         catch (const std::exception& ex)
         {
            logger_->warn("Invalid placefile entry: {}", ex.what());
         }
      }
   }
}

void PlacefileManager::Impl::SavePlacefileSettings()
{
   if (!placefileSettingsRead_)
   {
      return;
   }
   logger_->info("Saving placefile settings");

   std::shared_lock lock {placefileRecordLock_};
   auto             placefileJson = boost::json::value_from(placefileRecords_);
   scwx::util::json::WriteJsonFile(placefileSettingsPath_, placefileJson);
}

void PlacefileManager::WritePlacefileSettings(std::ostream& os)
{
   const std::shared_lock lock {p->placefileRecordLock_};
   auto placefileJson = boost::json::value_from(p->placefileRecords_);
   scwx::util::json::WriteJsonStream(os, placefileJson);
}

void PlacefileManager::SetRadarSite(
   std::shared_ptr<config::RadarSite> radarSite)
{
   if (p->radarSite_ == radarSite || radarSite == nullptr)
   {
      // No action needed
      return;
   }

   logger_->debug("SetRadarSite: {}", radarSite->id());

   p->radarSite_ = radarSite;

   // Update all enabled records
   std::shared_lock lock(p->placefileRecordLock_);
   for (auto& record : p->placefileRecords_)
   {
      if (record->enabled_)
      {
         record->UpdateAsync();
      }
   }
}

void PlacefileManager::AddUrl(const std::string& urlString,
                              const std::string& title,
                              bool               enabled,
                              bool               thresholded)
{
   std::string normalizedUrl = util::network::NormalizeUrl(urlString);

   std::unique_lock lock(p->placefileRecordLock_);

   // Determine if the placefile has been loaded previously
   auto it = std::find_if(p->placefileRecords_.begin(),
                          p->placefileRecords_.end(),
                          [&normalizedUrl](auto& record)
                          { return record->name_ == normalizedUrl; });
   if (it != p->placefileRecords_.end())
   {
      logger_->debug("Placefile already added: {}", normalizedUrl);
      return;
   }

   // Placefile is new, proceed with adding
   logger_->info("AddUrl: {}", normalizedUrl);

   // Add an empty placefile record for the new URL
   auto& record =
      p->placefileRecords_.emplace_back(std::make_shared<Impl::PlacefileRecord>(
         p.get(), normalizedUrl, nullptr, title, enabled, thresholded));
   p->placefileRecordMap_.insert_or_assign(normalizedUrl, record);

   lock.unlock();

   if (enabled)
   {
      Q_EMIT PlacefileEnabled(normalizedUrl, record->enabled_);
   }

   Q_EMIT PlacefileUpdated(normalizedUrl);

   // Queue a placefile update, either if enabled, or if we don't know the title
   if (enabled || title.empty())
   {
      record->UpdateAsync();
   }
}

void PlacefileManager::RemoveUrl(const std::string& urlString)
{
   std::unique_lock lock(p->placefileRecordLock_);

   // Determine if the placefile has been loaded previously
   auto it = std::find_if(p->placefileRecords_.begin(),
                          p->placefileRecords_.end(),
                          [&urlString](auto& record)
                          { return record->name_ == urlString; });
   if (it == p->placefileRecords_.end())
   {
      logger_->debug("Placefile doesn't exist: {}", urlString);
      return;
   }

   // Placefile exists, proceed with removing
   logger_->info("RemoveUrl: {}", urlString);

   // Remove record
   p->placefileRecords_.erase(it);
   p->placefileRecordMap_.erase(urlString);

   lock.unlock();

   Q_EMIT PlacefileRemoved(urlString);
}

void PlacefileManager::Refresh(const std::string& name)
{
   std::shared_lock lock {p->placefileRecordLock_};

   auto it = p->placefileRecordMap_.find(name);
   if (it != p->placefileRecordMap_.cend())
   {
      it->second->UpdateAsync();
   }
}

void PlacefileManager::Impl::PlacefileRecord::Update()
{
   logger_->debug("Update: {}", name_);

   // Take unique lock before refreshing
   std::unique_lock lock {refreshMutex_};

   // Make a copy of name in the event it changes.
   const std::string name {name_};

   std::shared_ptr<gr::Placefile> updatedPlacefile {};

   QUrl url = QUrl::fromUserInput(QString::fromStdString(name));
   if (url.isLocalFile())
   {
      updatedPlacefile = gr::Placefile::Load(name);

      if (updatedPlacefile == nullptr)
      {
         logger_->error("Local placefile not found: {}", name);
      }
   }
   else
   {
      if (name == kGoesGlmLightningUrl_)
      {
         updatedPlacefile = BuildGoesGlmPlacefile(name);
         if (updatedPlacefile == nullptr && enabled_)
         {
            logger_->warn("GOES GLM lightning update returned no data");
         }
      }
      else
      {
         std::string decodedUrl {name};
         auto        queryPos = decodedUrl.find('?');
         if (queryPos != std::string::npos)
         {
            decodedUrl.erase(queryPos);
         }

         if (p->radarSite_ == nullptr)
         {
            // Wait to process until a radar site is selected
            return;
         }

         auto dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();

         // Specify parameters
         auto parameters = cpr::Parameters {
            {"version", "1.5"}, // Placefile Version Supported
            {"dpi", fmt::format("{:0.0f}", dpi)},
            {"lat", fmt::format("{:0.3f}", p->radarSite_->latitude())},
            {"lon", fmt::format("{:0.3f}", p->radarSite_->longitude())}};

         // Iterate through each query parameter in the URL
         if (url.hasQuery())
         {
            auto query =
               url.query(QUrl::ComponentFormattingOption::PrettyDecoded)
                  .toStdString();

            boost::char_separator<char> delimiter("&");
            boost::tokenizer            tokens(query, delimiter);

            for (auto& token : tokens)
            {
               std::vector<std::string> split {};
               boost::split(split, token, boost::is_any_of("="));
               if (split.size() >= 2)
               {
                  // Token is a key=value parameter
                  parameters.Add({split[0], split[1]});
               }
               else
               {
                  // Token is a single key with no value
                  parameters.Add({token, {}});
               }
            }
         }

         // Send HTTP GET request
         auto response =
            cpr::Get(cpr::Url {decodedUrl},
                     network::cpr::GetHeader(),
                     parameters,
                     network::cpr::GetDefaultTimeout(),
                     network::cpr::GetDefaultConnectTimeout(),
                     network::cpr::GetDefaultLowSpeed(),
                     network::cpr::GetDefaultProgressCallback(enabled_));

         if (cpr::status::is_success(response.status_code))
         {
            std::istringstream responseBody {response.text};
            updatedPlacefile = gr::Placefile::Load(name, responseBody);
         }
         else if (response.status_code == 0 && enabled_)
         {
            logger_->error("Error loading placefile: {} ({})",
                           decodedUrl,
                           response.error.message);
         }
         else if (enabled_)
         {
            logger_->error("Error loading placefile: {} ({})",
                           decodedUrl,
                           response.status_line);
         }
         else
         {
            logger_->debug("Request cancelled, shutting down");
         }
      }
   }

   if (updatedPlacefile != nullptr)
   {
      // Load placefile resources
      auto newFonts  = Impl::LoadFontResources(updatedPlacefile);
      auto newImages = Impl::LoadImageResources(updatedPlacefile);

      // Check the name matches, in case the name updated
      if (name_ == name)
      {
         // Update the placefile
         placefile_      = updatedPlacefile;
         title_          = placefile_->title();
         lastUpdateTime_ = std::chrono::system_clock::now();
         failureCount_   = 0;

         // Update font resources
         {
            std::unique_lock fontsLock {fontsMutex_};
            fonts_.swap(newFonts);
            newFonts.clear();
         }

         // Update image resources
         images_.swap(newImages);
         newImages.clear();

         if (p->radarSite_ != nullptr)
         {
            lastRadarSite_ = p->radarSite_->id();
         }

         // Notify slots of the placefile update
         Q_EMIT p->self_->PlacefileUpdated(name);
      }

      // Update refresh timer
      ScheduleRefresh();
   }
   else if (enabled_)
   {
      using namespace std::chrono_literals;

      ++failureCount_;

      // Update refresh timer if the file failed to load, in case it is able to
      // be resolved later
      if (url.isLocalFile())
      {
         ScheduleRefresh(10s);
      }
      else
      {
         // Start attempting to refresh at 15 seconds, and start backing off
         // until retrying every 60 seconds
         ScheduleRefresh(
            std::min<std::chrono::seconds>(15s * failureCount_, 60s));
      }
   }
}

void PlacefileManager::Impl::PlacefileRecord::ScheduleRefresh()
{
   using namespace std::chrono_literals;

   if (!enabled_ || !refresh_enabled())
   {
      // Refresh is disabled
      return;
   }

   std::unique_lock lock {timerMutex_};

   auto nextUpdateTime      = lastUpdateTime_ + refresh_time();
   auto timeUntilNextUpdate = nextUpdateTime - std::chrono::system_clock::now();

   ScheduleRefresh(timeUntilNextUpdate);
}

void PlacefileManager::Impl::PlacefileRecord::ScheduleRefresh(
   const std::chrono::system_clock::duration timeUntilNextUpdate)
{
   logger_->debug(
      "Scheduled refresh in {:%M:%S} ({})",
      std::chrono::duration_cast<std::chrono::seconds>(timeUntilNextUpdate),
      name_);

   refreshTimer_.expires_after(timeUntilNextUpdate);
   refreshTimer_.async_wait(
      [this](const boost::system::error_code& e)
      {
         if (e == boost::asio::error::operation_aborted)
         {
            logger_->debug("Refresh timer cancelled");
         }
         else if (e != boost::system::errc::success)
         {
            logger_->warn("Refresh timer error: {}", e.message());
         }
         else
         {
            UpdateAsync();
         }
      });
}

void PlacefileManager::Impl::PlacefileRecord::CancelRefresh()
{
   std::unique_lock lock {timerMutex_};
   refreshTimer_.cancel();
}

void PlacefileManager::Impl::PlacefileRecord::UpdateAsync()
{
   boost::asio::post(threadPool_,
                     [this]()
                     {
                        try
                        {
                           Update();
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

std::shared_ptr<PlacefileManager> PlacefileManager::Instance()
{
   static std::weak_ptr<PlacefileManager> placefileManagerReference_ {};
   static std::mutex                      instanceMutex_ {};

   std::unique_lock lock(instanceMutex_);

   std::shared_ptr<PlacefileManager> placefileManager =
      placefileManagerReference_.lock();

   if (placefileManager == nullptr)
   {
      placefileManager           = std::make_shared<PlacefileManager>();
      placefileManagerReference_ = placefileManager;
   }

   return placefileManager;
}

PlacefileManager::FontMap PlacefileManager::Impl::LoadFontResources(
   const std::shared_ptr<gr::Placefile>& placefile)
{
   FontMap imGuiFonts {};
   auto    fonts = placefile->fonts();

   for (auto& font : fonts)
   {
      units::font_size::pixels<double> size {font.second->pixels_};
      std::vector<std::string>         styles {};

      if (font.second->IsBold())
      {
         styles.push_back("bold");
      }
      if (font.second->IsItalic())
      {
         styles.push_back("italic");
      }

      auto imGuiFont =
         FontManager::Instance().LoadImGuiFont(font.second->face_, styles);
      imGuiFonts.emplace(
         font.first,
         std::make_pair(std::move(imGuiFont), FontManager::ImFontSize(size)));
   }

   return imGuiFonts;
}

std::vector<std::shared_ptr<boost::gil::rgba8_image_t>>
PlacefileManager::Impl::LoadImageResources(
   const std::shared_ptr<gr::Placefile>& placefile)
{
   const auto iconFiles = placefile->icon_files();
   const auto drawItems = placefile->GetDrawItems();

   const QUrl baseUrl =
      QUrl::fromUserInput(QString::fromStdString(placefile->name()));

   std::vector<std::string> urlStrings {};
   urlStrings.reserve(iconFiles.size());

   // Resolve Icon Files
   std::transform(iconFiles.cbegin(),
                  iconFiles.cend(),
                  std::back_inserter(urlStrings),
                  [&baseUrl](auto& iconFile)
                  {
                     // Resolve target URL relative to base URL
                     QString filePath =
                        QString::fromStdString(iconFile->filename_);
                     QUrl fileUrl = QUrl(QDir::fromNativeSeparators(filePath));
                     QUrl resolvedUrl = baseUrl.resolved(fileUrl);

                     return resolvedUrl.toString().toStdString();
                  });

   // Resolve Image Files
   for (auto& di : drawItems)
   {
      switch (di->itemType_)
      {
      case gr::Placefile::ItemType::Image:
      case gr::Placefile::ItemType::ImageXY:
      {
         const std::string& imageFile =
            std::static_pointer_cast<gr::Placefile::ImageBaseDrawItem>(di)
               ->imageFile_;

         QString     filePath    = QString::fromStdString(imageFile);
         QUrl        fileUrl     = QUrl(QDir::fromNativeSeparators(filePath));
         QUrl        resolvedUrl = baseUrl.resolved(fileUrl);
         std::string urlString   = resolvedUrl.toString().toStdString();

         if (std::find(urlStrings.cbegin(), urlStrings.cend(), urlString) ==
             urlStrings.cend())
         {
            urlStrings.push_back(urlString);
         }
         break;
      }

      default:
         break;
      }
   }

   return ResourceManager::LoadImageResources(urlStrings);
}

} // namespace scwx::qt::manager
