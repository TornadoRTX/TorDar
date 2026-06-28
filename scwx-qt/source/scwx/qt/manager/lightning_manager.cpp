#include <scwx/qt/manager/lightning_manager.hpp>

#include <scwx/qt/main/application.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/util/geographic_lib.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/util/logger.hpp>

#include <cpr/cpr.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

#if SCWX_HAS_NETCDF
#   include <netcdf.h>
#endif

#include <algorithm>
#include <array>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <QGuiApplication>
#include <QScreen>
#include <QString>
#include <QUrl>
#include <boost/tokenizer.hpp>

namespace scwx::qt::manager
{

namespace
{

static const std::string logPrefix_ = "scwx::qt::manager::lightning_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static const std::string kLegacyLightningTitle_ = "Legacy Lightning";
static const std::string kLegacyLightningUrl_ =
   "https://www.freelightning.com/hub/"
   "placefile.php?request=10213|10454|138624046|10463|10369|10644|0|84764|1";
static const std::string kLightningTitle_ = "GOES GLM Lightning (AWS)";
static const std::string kLightningUrl_ =
   "https://noaa-goes19.s3.amazonaws.com/index.html";
static const std::string kLightningPrimaryBucketUrl_ =
   "https://noaa-goes19.s3.amazonaws.com";
static const std::string kLightningFallbackBucketUrl_ =
   "https://noaa-goes18.s3.amazonaws.com";
static constexpr std::chrono::minutes kLightningRetention_ {15};
static constexpr std::chrono::seconds kLightningRefreshSeconds_ {30};
static constexpr int                  kTmYearEpochOffset_ {1900};
static constexpr std::size_t          kFileStartYearGroup_ {1};
static constexpr std::size_t          kFileStartDayGroup_ {2};
static constexpr std::size_t          kFileStartHourGroup_ {3};
static constexpr std::size_t          kFileStartMinuteGroup_ {4};
static constexpr std::size_t          kFileStartSecondGroup_ {5};
static constexpr int                  kHoursPerDay_ {24};
static constexpr std::size_t          kMaxGlmFilesToDownload_ {20};
static constexpr std::size_t          kExpectedLightningPoints_ {5000};
static constexpr std::uint32_t        kOverlapHashSeed_ {0x9e3779b9u};
static constexpr std::uint32_t        kOverlapHashLeftShift_ {6u};
static constexpr std::uint32_t        kOverlapHashRightShift_ {2u};

struct GlmLightningPoint
{
   double                                latitude_ {};
   double                                longitude_ {};
   std::chrono::system_clock::time_point time_ {};
};

struct GlmLightningPointKey
{
   std::int64_t latitude_ {};
   std::int64_t longitude_ {};

   bool operator==(const GlmLightningPointKey&) const = default;
};

struct GlmLightningPointKeyHash
{
   std::size_t operator()(const GlmLightningPointKey& key) const noexcept
   {
      const std::size_t latHash = std::hash<std::int64_t> {}(key.latitude_);
      const std::size_t lonHash = std::hash<std::int64_t> {}(key.longitude_);
      return latHash ^ (lonHash + kOverlapHashSeed_ +
                        (latHash << kOverlapHashLeftShift_) +
                        (latHash >> kOverlapHashRightShift_));
   }
};

static GlmLightningPointKey MakeGlmLightningPointKey(double latitude,
                                                     double longitude)
{
   static constexpr double kGlmCoordinateQuantization_ {100000.0};

   return {static_cast<std::int64_t>(
              std::llround(latitude * kGlmCoordinateQuantization_)),
           static_cast<std::int64_t>(
              std::llround(longitude * kGlmCoordinateQuantization_))};
}

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
   tm.tm_year =
      std::stoi(match[kFileStartYearGroup_].str()) - kTmYearEpochOffset_;
   tm.tm_mon  = 0;
   tm.tm_mday = 1;
   tm.tm_hour = std::stoi(match[kFileStartHourGroup_].str());
   tm.tm_min  = std::stoi(match[kFileStartMinuteGroup_].str());
   tm.tm_sec  = std::stoi(match[kFileStartSecondGroup_].str());

   const auto jan1      = UtcTmToTimePoint(tm);
   const int  dayOfYear = std::stoi(match[kFileStartDayGroup_].str());
   *t = jan1 + std::chrono::hours(kHoursPerDay_ * (dayOfYear - 1));
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
FetchGlmKeysFromBucket(const std::string&                           bucketUrl,
                       const std::chrono::system_clock::time_point& nowUtc)
{
   std::vector<std::string> keys {};
   const auto               tt = std::chrono::system_clock::to_time_t(nowUtc);
   std::tm                  tmUtc {};
#if defined(_WIN32)
   gmtime_s(&tmUtc, &tt);
#else
   gmtime_r(&tt, &tmUtc);
#endif

   const int minCurrentHour = tmUtc.tm_min;
   const int minRetention   = static_cast<int>(kLightningRetention_.count());

   for (int hourOffset = 0;
        hourOffset >= (minCurrentHour < minRetention ? -1 : 0);
        --hourOffset)
   {
      const auto hourTime = nowUtc + std::chrono::hours(hourOffset);
      const auto hourTt   = std::chrono::system_clock::to_time_t(hourTime);
      std::tm    hourTm {};
#if defined(_WIN32)
      gmtime_s(&hourTm, &hourTt);
#else
      gmtime_r(&hourTt, &hourTm);
#endif

      const std::string prefix =
         fmt::format("GLM-L2-LCFA/{:04d}/{:03d}/{:02d}/",
                     hourTm.tm_year + kTmYearEpochOffset_,
                     hourTm.tm_yday + 1,
                     hourTm.tm_hour);

      auto response = cpr::Get(cpr::Url {bucketUrl},
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

   std::vector<int> dimIds(static_cast<std::size_t>(ndims), 0);
   if (nc_inq_vardimid(ncid, varid, dimIds.data()) != NC_NOERR)
   {
      return false;
   }

   size_t count {1u};
   for (int dimId : dimIds)
   {
      size_t dimLen {};
      if (nc_inq_dimlen(ncid, dimId, &dimLen) != NC_NOERR || dimLen == 0u)
      {
         return false;
      }
      count *= dimLen;
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
      std::string        coverageStart {buffer.data()};
      std::istringstream ss {coverageStart};
      ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
      if (ss.fail())
      {
         ss.clear();
         ss.str(coverageStart);
         ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
      }
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

   size_t     count          = 0u;
   const bool hasTimeOffsets = !offsets.empty();
   if (hasTimeOffsets)
   {
      count = std::min(latitudes.size(),
                       std::min(longitudes.size(), offsets.size()));
   }
   else
   {
      count = std::min(latitudes.size(), longitudes.size());
      logger_->warn(
         "GLM file missing expected time offset variable, using file "
         "coverage start time for all points: {}",
         ncFile.string());
   }
   points.reserve(count);

   for (size_t i = 0; i < count; ++i)
   {
      points.push_back(
         {latitudes[i],
          longitudes[i],
          hasTimeOffsets ?
             baseTime + std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::duration<double>(offsets[i])) :
             baseTime});
   }
#else
   (void) ncFile;
   logger_->warn(
      "GOES GLM support compiled without NetCDF; no GLM points can be decoded");
#endif

   return points;
}

static void AppendGlmFileCandidates(
   const std::string&                                              bucketUrl,
   const std::chrono::system_clock::time_point&                    nowUtc,
   const std::chrono::system_clock::time_point&                    cutoff,
   std::vector<std::tuple<std::string,
                          std::string,
                          std::chrono::system_clock::time_point>>& candidates)
{
   for (const auto& key : FetchGlmKeysFromBucket(bucketUrl, nowUtc))
   {
      std::chrono::system_clock::time_point startTime {};
      if (key.ends_with(".nc") && ParseGlmFileStartTime(key, &startTime) &&
          startTime >= cutoff)
      {
         candidates.emplace_back(bucketUrl, key, startTime);
      }
   }
}

static std::shared_ptr<gr::Placefile>
BuildLightningPlacefile(const std::string&                        placefileName,
                        const std::shared_ptr<config::RadarSite>& radarSite,
                        const std::optional<float>&               radarRangeKm)
{
   const bool radarFilteringEnabled = radarSite != nullptr &&
                                      radarRangeKm.has_value() &&
                                      radarRangeKm.value() > 0.0f;
   if (!radarFilteringEnabled)
   {
      logger_->debug("GOES GLM: waiting for radar scan range");
      return nullptr;
   }

   std::ostringstream pf {};
   pf << "Title: " << kLightningTitle_ << "\n";
   pf << "RefreshSeconds: 30\n";
   pf << "Threshold: 999\n";
   pf << "IconFile: 1, 8, 8, 4, 4, "
         "\"qrc:/res/icons/flaticon/lightning.svg\"\n";

   const auto nowUtc = UtcNowMinute();
   const auto cutoff = nowUtc - kLightningRetention_;

   std::vector<std::tuple<std::string,
                          std::string,
                          std::chrono::system_clock::time_point>>
      candidates {};

   AppendGlmFileCandidates(
      kLightningPrimaryBucketUrl_, nowUtc, cutoff, candidates);

   if (candidates.empty())
   {
      logger_->info("GOES GLM: GOES-19 unavailable, falling back to GOES-18");
      AppendGlmFileCandidates(
         kLightningFallbackBucketUrl_, nowUtc, cutoff, candidates);
   }

   if (candidates.empty())
   {
      logger_->warn("GOES GLM: no recent files found in the last {} minutes",
                    kLightningRetention_.count());
      std::istringstream stream {pf.str()};
      return gr::Placefile::Load(placefileName, stream);
   }

   std::sort(candidates.begin(),
             candidates.end(),
             [](const auto& lhs, const auto& rhs)
             { return std::get<2>(lhs) > std::get<2>(rhs); });

   if (candidates.size() > kMaxGlmFilesToDownload_)
   {
      candidates.resize(kMaxGlmFilesToDownload_);
   }

   std::vector<GlmLightningPoint> points {};
   points.reserve(kExpectedLightningPoints_);
   const units::length::meters<double> radarRangeMeters =
      units::length::meters<double> {radarRangeKm.value() * 1000.0};

   std::size_t decodedPointCount        = 0u;
   std::size_t skippedOutsideRadarRange = 0u;

   for (const auto& [bucketUrl, key, fileStart] : candidates)
   {
      auto response = cpr::Get(cpr::Url {fmt::format("{}/{}", bucketUrl, key)},
                               network::cpr::GetHeader(),
                               network::cpr::GetDefaultTimeout(),
                               network::cpr::GetDefaultConnectTimeout(),
                               network::cpr::GetDefaultLowSpeed());

      if (!cpr::status::is_success(response.status_code))
      {
         logger_->debug("GOES GLM: failed to download {} (status {})",
                        key,
                        response.status_code);
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

      for (auto& point : filePoints)
      {
         point.time_ = fileStart;
      }

      decodedPointCount += filePoints.size();

      for (const auto& point : filePoints)
      {
         const units::length::meters<double> distance =
            util::GeographicLib::GetDistance(radarSite->latitude(),
                                             radarSite->longitude(),
                                             point.latitude_,
                                             point.longitude_);

         if (distance > radarRangeMeters)
         {
            ++skippedOutsideRadarRange;
            continue;
         }

         points.push_back(point);
      }
   }

   logger_->debug("GOES GLM: decoded {} raw points from {} files",
                  decodedPointCount,
                  candidates.size());
   logger_->debug("GOES GLM: skipped {} points outside radar range",
                  skippedOutsideRadarRange);

   if (points.empty())
   {
      logger_->debug("GOES GLM: emitted 0 points after filtering");
      std::istringstream stream {pf.str()};
      return gr::Placefile::Load(placefileName, stream);
   }

   std::unordered_set<GlmLightningPointKey, GlmLightningPointKeyHash>
      emittedPointKeys {};
   emittedPointKeys.reserve(points.size());

   std::size_t skippedDuplicatePoints = 0u;
   std::size_t emittedPoints          = 0u;

   for (const auto& p : points)
   {
      if (p.time_ < cutoff || p.time_ > nowUtc)
      {
         continue;
      }

      const GlmLightningPointKey pointKey =
         MakeGlmLightningPointKey(p.latitude_, p.longitude_);
      if (emittedPointKeys.contains(pointKey))
      {
         ++skippedDuplicatePoints;
         continue;
      }
      emittedPointKeys.insert(pointKey);

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
      ++emittedPoints;
   }

   if (skippedDuplicatePoints > 0u)
   {
      logger_->debug("GOES GLM: suppressed {} duplicate points",
                     skippedDuplicatePoints);
   }

   logger_->debug("GOES GLM: unrendered {} points total",
                  skippedOutsideRadarRange + skippedDuplicatePoints);
   logger_->debug("GOES GLM: emitted {} points after filtering", emittedPoints);

   std::istringstream stream {pf.str()};
   return gr::Placefile::Load(placefileName, stream);
}

static std::shared_ptr<gr::Placefile> BuildLegacyLightningPlacefile(
   const std::string&                        placefileName,
   const std::shared_ptr<config::RadarSite>& radarSite)
{
   if (radarSite == nullptr)
   {
      logger_->debug("Legacy lightning: waiting for radar site");
      return nullptr;
   }

   QUrl const url =
      QUrl::fromUserInput(QString::fromStdString(kLegacyLightningUrl_));

   std::string decodedUrl {kLegacyLightningUrl_};
   auto        queryPos = decodedUrl.find('?');
   if (queryPos != std::string::npos)
   {
      decodedUrl.erase(queryPos);
   }

   auto dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();

   auto parameters =
      cpr::Parameters {{"version", "1.5"}, // Placefile Version Supported
                       {"dpi", fmt::format("{:0.0f}", dpi)},
                       {"lat", fmt::format("{:0.3f}", radarSite->latitude())},
                       {"lon", fmt::format("{:0.3f}", radarSite->longitude())}};

   if (url.hasQuery())
   {
      auto query = url.query(QUrl::ComponentFormattingOption::PrettyDecoded)
                      .toStdString();

      boost::char_separator<char> const delimiter("&");
      boost::tokenizer const            tokens(query, delimiter);

      for (auto& token : tokens)
      {
         std::vector<std::string> split {};
         boost::split(split, token, boost::is_any_of("="));
         if (split.size() >= 2)
         {
            parameters.Add({split[0], split[1]});
         }
         else
         {
            parameters.Add({token, {}});
         }
      }
   }

   auto response = cpr::Get(cpr::Url {decodedUrl},
                            network::cpr::GetHeader(),
                            parameters,
                            network::cpr::GetDefaultTimeout(),
                            network::cpr::GetDefaultConnectTimeout(),
                            network::cpr::GetDefaultLowSpeed(),
                            network::cpr::GetDefaultProgressCallback(true));

   if (!cpr::status::is_success(response.status_code))
   {
      logger_->warn("Legacy lightning: error loading placefile: {} ({})",
                    decodedUrl,
                    response.status_line);
      return nullptr;
   }

   std::istringstream responseBody {response.text};
   return gr::Placefile::Load(placefileName, responseBody);
}

} // namespace

class LightningManager::Impl
{
public:
   explicit Impl(LightningManager* self) : self_ {self}
   {
      settings::GeneralSettings::Instance()
         .legacy_lightning_enabled()
         .RegisterValueChangedCallback([this](const bool& /* enabled */)
                                       { SyncEnabled(); });
      settings::GeneralSettings::Instance()
         .goes_glm_lightning_enabled()
         .RegisterValueChangedCallback([this](const bool& /* enabled */)
                                       { SyncEnabled(); });

      SyncEnabled();
   }

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;
   ~Impl()                       = default;

   void SetRadarSite(std::shared_ptr<config::RadarSite> radarSite);
   void SetRadarScanRange(std::optional<float> radarRangeKm);
   void NotifyMapZoom(double zoom);
   void RefreshAsync();
   void Refresh();
   void SyncEnabled();

   void ScheduleRefresh();
   void ScheduleRefresh(
      const std::chrono::system_clock::duration timeUntilNextUpdate);
   void CancelRefresh();

   LightningManager*                     self_;
   std::shared_ptr<config::RadarSite>    radarSite_ {};
   std::optional<float>                  radarRangeKm_ {};
   std::shared_ptr<gr::Placefile>        placefile_ {};
   bool                                  legacyEnabled_ {false};
   bool                                  enabled_ {false};
   std::optional<double>                 lastZoom_ {};
   boost::asio::thread_pool              threadPool_ {1u};
   boost::asio::steady_timer             refreshTimer_ {threadPool_};
   std::mutex                            refreshMutex_ {};
   std::mutex                            timerMutex_ {};
   std::chrono::system_clock::time_point lastUpdateTime_ {};
   std::size_t                           failureCount_ {};
};

void LightningManager::Impl::SetRadarSite(
   std::shared_ptr<config::RadarSite> radarSite)
{
   std::unique_lock const lock {refreshMutex_};

   if (radarSite_ == radarSite)
   {
      return;
   }

   radarSite_ = std::move(radarSite);

   if (radarSite_ == nullptr)
   {
      radarRangeKm_.reset();
      placefile_.reset();
      Q_EMIT self_->LightningUpdated();
      CancelRefresh();
      return;
   }

   radarRangeKm_.reset();

   RefreshAsync();
}

void LightningManager::Impl::SetRadarScanRange(
   std::optional<float> radarRangeKm)
{
   std::unique_lock const lock {refreshMutex_};

   if (radarRangeKm_ == radarRangeKm)
   {
      return;
   }

   radarRangeKm_ = radarRangeKm;

   if (legacyEnabled_ && !enabled_)
   {
      RefreshAsync();
      return;
   }

   if (!enabled_ || radarSite_ == nullptr || !radarRangeKm_.has_value())
   {
      placefile_.reset();
      Q_EMIT self_->LightningUpdated();
      CancelRefresh();
      return;
   }

   RefreshAsync();
}

void LightningManager::Impl::NotifyMapZoom(double zoom)
{
   if (lastZoom_.has_value() && lastZoom_.value() == zoom)
   {
      return;
   }

   lastZoom_ = zoom;
   Q_EMIT self_->ViewUpdated();
}

void LightningManager::Impl::RefreshAsync()
{
   boost::asio::post(threadPool_,
                     [this]()
                     {
                        try
                        {
                           Refresh();
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

void LightningManager::Impl::Refresh()
{
   std::unique_lock const lock {refreshMutex_};

   if ((!enabled_ && !legacyEnabled_) || radarSite_ == nullptr)
   {
      if (placefile_ != nullptr)
      {
         placefile_.reset();
         Q_EMIT self_->LightningUpdated();
      }
      return;
   }

   std::shared_ptr<gr::Placefile> updatedPlacefile {};
   if (enabled_ && radarRangeKm_.has_value() && radarRangeKm_.value() > 0.0f)
   {
      updatedPlacefile =
         BuildLightningPlacefile(kLightningUrl_, radarSite_, radarRangeKm_);
   }
   else if (legacyEnabled_)
   {
      updatedPlacefile =
         BuildLegacyLightningPlacefile(kLegacyLightningUrl_, radarSite_);
   }

   if (updatedPlacefile != nullptr)
   {
      placefile_      = std::move(updatedPlacefile);
      lastUpdateTime_ = std::chrono::system_clock::now();
      failureCount_   = 0u;
      Q_EMIT self_->LightningUpdated();
      ScheduleRefresh();
   }
   else
   {
      ++failureCount_;
      placefile_.reset();
      Q_EMIT self_->LightningUpdated();
      using namespace std::chrono_literals;
      ScheduleRefresh(std::max<std::chrono::seconds>(15s * failureCount_, 60s));
   }
}

void LightningManager::Impl::SyncEnabled()
{
   std::unique_lock const lock {refreshMutex_};
   legacyEnabled_ = settings::GeneralSettings::Instance()
                       .legacy_lightning_enabled()
                       .GetValue();
   enabled_ = settings::GeneralSettings::Instance()
                 .goes_glm_lightning_enabled()
                 .GetValue();

   if (!enabled_ && !legacyEnabled_)
   {
      CancelRefresh();
      if (placefile_ != nullptr)
      {
         placefile_.reset();
         Q_EMIT self_->LightningUpdated();
      }
      return;
   }

   if (radarSite_ != nullptr && ((enabled_ && radarRangeKm_.has_value() &&
                                  radarRangeKm_.value() > 0.0f) ||
                                 legacyEnabled_))
   {
      RefreshAsync();
   }
}

void LightningManager::Impl::ScheduleRefresh()
{
   using namespace std::chrono_literals;

   if ((!enabled_ && !legacyEnabled_) ||
       (enabled_ && !radarRangeKm_.has_value()))
   {
      return;
   }

   std::unique_lock const lock {timerMutex_};
   auto nextUpdateTime      = lastUpdateTime_ + kLightningRefreshSeconds_;
   auto timeUntilNextUpdate = nextUpdateTime - std::chrono::system_clock::now();
   ScheduleRefresh(timeUntilNextUpdate);
}

void LightningManager::Impl::ScheduleRefresh(
   const std::chrono::system_clock::duration timeUntilNextUpdate)
{
   logger_->debug(
      "Scheduled refresh in {:%M:%S} (GOES GLM)",
      std::chrono::duration_cast<std::chrono::seconds>(timeUntilNextUpdate));

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
            RefreshAsync();
         }
      });
}

void LightningManager::Impl::CancelRefresh()
{
   std::unique_lock const lock {timerMutex_};
   refreshTimer_.cancel();
}

LightningManager::LightningManager() : p(std::make_unique<Impl>(this)) {}
LightningManager::~LightningManager() = default;

std::shared_ptr<LightningManager> LightningManager::Instance()
{
   static std::weak_ptr<LightningManager> lightningManagerReference_ {};
   static std::mutex                      instanceMutex_ {};

   std::unique_lock const lock(instanceMutex_);

   std::shared_ptr<LightningManager> lightningManager =
      lightningManagerReference_.lock();

   if (lightningManager == nullptr)
   {
      lightningManager           = std::make_shared<LightningManager>();
      lightningManagerReference_ = lightningManager;
   }

   return lightningManager;
}

void LightningManager::SetRadarSite(
   std::shared_ptr<config::RadarSite> radarSite)
{
   p->SetRadarSite(std::move(radarSite));
}

void LightningManager::SetRadarScanRange(std::optional<float> radarRangeKm)
{
   p->SetRadarScanRange(radarRangeKm);
}

void LightningManager::NotifyMapZoom(double zoom)
{
   p->NotifyMapZoom(zoom);
}

std::shared_ptr<gr::Placefile> LightningManager::placefile() const
{
   const std::unique_lock lock {p->refreshMutex_};
   return p->placefile_;
}

bool LightningManager::IsLightningPlacefile(const std::string& name)
{
   return name == kLightningUrl_ || name == kLegacyLightningUrl_;
}

} // namespace scwx::qt::manager
