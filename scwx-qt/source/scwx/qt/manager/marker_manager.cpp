#include <scwx/qt/manager/marker_manager.hpp>
#include <scwx/qt/types/marker_types.hpp>
#include <scwx/qt/util/color.hpp>
#include <scwx/qt/util/json.hpp>
#include <scwx/qt/util/texture_atlas.hpp>
#include <scwx/qt/main/application.hpp>
#include <scwx/qt/manager/resource_manager.hpp>
#include <scwx/util/logger.hpp>

#include <filesystem>
#include <shared_mutex>
#include <vector>
#include <string>
#include <unordered_map>

#include <QStandardPaths>
#include <boost/json.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

namespace scwx
{
namespace qt
{
namespace manager
{

static const std::string logPrefix_ = "scwx::qt::manager::marker_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static const std::string kNameName_      = "name";
static const std::string kLatitudeName_  = "latitude";
static const std::string kLongitudeName_ = "longitude";
static const std::string kIconName_      = "icon";
static const std::string kIconColorName_ = "icon-color";

static const std::string defaultIconName = "images/location-marker";

class MarkerManager::Impl
{
public:
   class MarkerRecord;

   explicit Impl(MarkerManager* self) : self_ {self} {}
   ~Impl() { threadPool_.join(); }

   std::string                                 markerSettingsPath_ {""};
   std::vector<std::shared_ptr<MarkerRecord>>  markerRecords_ {};
   std::unordered_map<types::MarkerId, size_t> idToIndex_ {};
   std::unordered_map<std::string, types::MarkerIconInfo> markerIcons_ {};

   MarkerManager* self_;

   boost::asio::thread_pool threadPool_ {1u};
   std::shared_mutex        markerRecordLock_ {};
   std::shared_mutex        markerIconsLock_ {};

   void                          InitializeMarkerSettings();
   void                          ReadMarkerSettings();
   void                          WriteMarkerSettings();
   std::shared_ptr<MarkerRecord> GetMarkerByName(const std::string& name);

   bool markerFileRead_ {false};

   void InitalizeIds();
   types::MarkerId NewId();
   types::MarkerId lastId_ {0};
};

class MarkerManager::Impl::MarkerRecord
{
public:
   MarkerRecord(const types::MarkerInfo& info) :
      markerInfo_ {info}
   {
   }

   const types::MarkerInfo& toMarkerInfo()
   {
      return markerInfo_;
   }

   types::MarkerInfo markerInfo_;

   friend void tag_invoke(boost::json::value_from_tag,
                          boost::json::value&                  jv,
                          const std::shared_ptr<MarkerRecord>& record)
   {
      jv = {{kNameName_, record->markerInfo_.name},
            {kLatitudeName_, record->markerInfo_.latitude},
            {kLongitudeName_, record->markerInfo_.longitude},
            {kIconName_, record->markerInfo_.iconName},
            {kIconColorName_,
             util::color::ToArgbString(record->markerInfo_.iconColor)}};
   }

   friend MarkerRecord tag_invoke(boost::json::value_to_tag<MarkerRecord>,
                                  const boost::json::value& jv)
   {
      static const boost::gil::rgba8_pixel_t defaultIconColor =
         util::color::ToRgba8PixelT("#ffff0000");

      const boost::json::object& jo = jv.as_object();

      std::string               iconName  = defaultIconName;
      boost::gil::rgba8_pixel_t iconColor = defaultIconColor;

      if (jo.contains(kIconName_) && jo.at(kIconName_).is_string())
      {
         iconName = boost::json::value_to<std::string>(jv.at(kIconName_));
      }

      if (jo.contains(kIconColorName_) && jo.at(kIconName_).is_string())
      {
         try
         {
            iconColor = util::color::ToRgba8PixelT(
               boost::json::value_to<std::string>(jv.at(kIconColorName_)));
         }
         catch (const std::exception& ex)
         {
            logger_->warn(
               "Could not parse color value in location-markers.json with the "
               "following exception: {}",
               ex.what());
         }
      }

      return {types::MarkerInfo(
         boost::json::value_to<std::string>(jv.at(kNameName_)),
         boost::json::value_to<double>(jv.at(kLatitudeName_)),
         boost::json::value_to<double>(jv.at(kLongitudeName_)),
         iconName,
         iconColor)};
   }
};

void MarkerManager::Impl::InitalizeIds()
{
   lastId_ = 0;
}

types::MarkerId MarkerManager::Impl::NewId()
{
   return ++lastId_;
}

void MarkerManager::Impl::InitializeMarkerSettings()
{
   std::string appDataPath {
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
         .toStdString()};

   if (!std::filesystem::exists(appDataPath))
   {
      if (!std::filesystem::create_directories(appDataPath))
      {
         logger_->error("Unable to create application data directory: \"{}\"",
                        appDataPath);
      }
   }

   markerSettingsPath_ = appDataPath + "/location-markers.json";
}

void MarkerManager::Impl::ReadMarkerSettings()
{
   logger_->info("Reading location marker settings");
   InitalizeIds();

   boost::json::value markerJson = nullptr;
   {
      const std::unique_lock lock(markerRecordLock_);

      // Determine if marker settings exists
      if (std::filesystem::exists(markerSettingsPath_))
      {
         markerJson = util::json::ReadJsonFile(markerSettingsPath_);
      }

      if (markerJson != nullptr && markerJson.is_array())
      {
         // For each marker entry
         auto& markerArray = markerJson.as_array();
         markerRecords_.reserve(markerArray.size());
         idToIndex_.reserve(markerArray.size());
         for (auto& markerEntry : markerArray)
         {
            try
            {
               auto record = boost::json::value_to<MarkerRecord>(markerEntry);

               const types::MarkerId id    = NewId();
               const size_t          index = markerRecords_.size();
               record.markerInfo_.id       = id;
               markerRecords_.emplace_back(
                  std::make_shared<MarkerRecord>(record.markerInfo_));
               idToIndex_.emplace(id, index);

               self_->add_icon(record.markerInfo_.iconName, true);
            }
            catch (const std::exception& ex)
            {
               logger_->warn("Invalid location marker entry: {}", ex.what());
            }
         }

         ResourceManager::BuildAtlas();

         logger_->debug("{} location marker entries", markerRecords_.size());
      }
   }

   markerFileRead_ = true;
   Q_EMIT self_->MarkersUpdated();
}

void MarkerManager::Impl::WriteMarkerSettings()
{
   if (!markerFileRead_)
   {
      return;
   }
   logger_->info("Saving location marker settings");

   const std::shared_lock lock(markerRecordLock_);
   auto markerJson = boost::json::value_from(markerRecords_);
   util::json::WriteJsonFile(markerSettingsPath_, markerJson);
}

std::shared_ptr<MarkerManager::Impl::MarkerRecord>
MarkerManager::Impl::GetMarkerByName(const std::string& name)
{
   for (auto& markerRecord : markerRecords_)
   {
      if (markerRecord->markerInfo_.name == name)
      {
         return markerRecord;
      }
   }

   return nullptr;
}

MarkerManager::MarkerManager() : p(std::make_unique<Impl>(this))
{
   static const std::vector<types::MarkerIconInfo> defaultMarkerIcons_ {
      types::MarkerIconInfo(types::ImageTexture::LocationMarker, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationPin, 6, 16),
      types::MarkerIconInfo(types::ImageTexture::LocationCrosshair, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationStar, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationBriefcase, -1, -1),
      types::MarkerIconInfo(
         types::ImageTexture::LocationBuildingColumns, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationBuilding, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationCaravan, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationHouse, -1, -1),
      types::MarkerIconInfo(types::ImageTexture::LocationTent, -1, -1),
   };

   p->InitializeMarkerSettings();

   boost::asio::post(p->threadPool_,
                     [this]()
                     {
                        try
                        {
                           // Read Marker settings on startup
                           main::Application::WaitForInitialization();
                           {
                              const std::unique_lock lock(p->markerIconsLock_);
                              p->markerIcons_.reserve(
                                 defaultMarkerIcons_.size());
                              for (auto& icon : defaultMarkerIcons_)
                              {
                                 p->markerIcons_.emplace(icon.name, icon);
                              }
                           }
                           p->ReadMarkerSettings();

                           Q_EMIT IconsReady();
                           Q_EMIT MarkersInitialized(p->markerRecords_.size());
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

MarkerManager::~MarkerManager()
{
   p->WriteMarkerSettings();
}

size_t MarkerManager::marker_count()
{
   return p->markerRecords_.size();
}

std::optional<types::MarkerInfo> MarkerManager::get_marker(types::MarkerId id)
{
   const std::shared_lock lock(p->markerRecordLock_);
   if (!p->idToIndex_.contains(id))
   {
      return {};
   }
   size_t index = p->idToIndex_[id];
   if (index >= p->markerRecords_.size())
   {
      logger_->warn("id in idToIndex_ but out of range!");
      return {};
   }
   std::shared_ptr<MarkerManager::Impl::MarkerRecord>& markerRecord =
      p->markerRecords_[index];
   return markerRecord->toMarkerInfo();
}

std::optional<size_t> MarkerManager::get_index(types::MarkerId id)
{
   const std::shared_lock lock(p->markerRecordLock_);
   if (!p->idToIndex_.contains(id))
   {
      return {};
   }
   return p->idToIndex_[id];
}

void MarkerManager::set_marker(types::MarkerId          id,
                               const types::MarkerInfo& marker)
{
   {
      const std::unique_lock lock(p->markerRecordLock_);
      if (!p->idToIndex_.contains(id))
      {
         return;
      }
      size_t index = p->idToIndex_[id];
      if (index >= p->markerRecords_.size())
      {
         logger_->warn("id in idToIndex_ but out of range!");
         return;
      }
      const std::shared_ptr<MarkerManager::Impl::MarkerRecord>& markerRecord =
         p->markerRecords_[index];
      markerRecord->markerInfo_    = marker;
      markerRecord->markerInfo_.id = id;

      add_icon(marker.iconName);
   }
   Q_EMIT MarkerChanged(id);
   Q_EMIT MarkersUpdated();
}

types::MarkerId MarkerManager::add_marker(const types::MarkerInfo& marker)
{
   types::MarkerId id;
   {
      const std::unique_lock lock(p->markerRecordLock_);
      id = p->NewId();
      size_t index = p->markerRecords_.size();
      p->idToIndex_.emplace(id, index);
      p->markerRecords_.emplace_back(std::make_shared<Impl::MarkerRecord>(marker));
      p->markerRecords_[index]->markerInfo_.id = id;

      add_icon(marker.iconName);
   }
   Q_EMIT MarkerAdded(id);
   Q_EMIT MarkersUpdated();
   return id;
}

void MarkerManager::remove_marker(types::MarkerId id)
{
   {
      const std::unique_lock lock(p->markerRecordLock_);
      if (!p->idToIndex_.contains(id))
      {
         return;
      }
      size_t index = p->idToIndex_[id];
      if (index >= p->markerRecords_.size())
      {
         logger_->warn("id in idToIndex_ but out of range!");
         return;
      }

      p->markerRecords_.erase(std::next(p->markerRecords_.begin(), index));
      p->idToIndex_.erase(id);

      for (auto& pair : p->idToIndex_)
      {
         if (pair.second > index)
         {
            pair.second -= 1;
         }
      }
   }

   Q_EMIT MarkerRemoved(id);
   Q_EMIT MarkersUpdated();
}

void MarkerManager::move_marker(size_t from, size_t to)
{
   {
      const std::unique_lock lock(p->markerRecordLock_);
      if (from >= p->markerRecords_.size() || to >= p->markerRecords_.size())
      {
         return;
      }
      std::shared_ptr<MarkerManager::Impl::MarkerRecord>& markerRecord =
         p->markerRecords_[from];

      if (from == to) {}
      else if (from < to)
      {
         for (size_t i = from; i < to; i++)
         {
            p->markerRecords_[i] = p->markerRecords_[i + 1];
         }
         p->markerRecords_[to] = markerRecord;
      }
      else
      {
         for (size_t i = from; i > to; i--)
         {
            p->markerRecords_[i] = p->markerRecords_[i - 1];
         }
         p->markerRecords_[to] = markerRecord;
      }
   }
   Q_EMIT MarkersUpdated();
}

void MarkerManager::for_each(std::function<MarkerForEachFunc> func)
{
   const std::shared_lock lock(p->markerRecordLock_);
   for (auto marker : p->markerRecords_)
   {
      func(marker->markerInfo_);
   }
}

void MarkerManager::add_icon(const std::string& name, bool startup)
{
   {
      const std::unique_lock lock(p->markerIconsLock_);
      if (p->markerIcons_.contains(name))
      {
         return;
      }
      const std::shared_ptr<boost::gil::rgba8_image_t> image =
         ResourceManager::LoadImageResource(name);

      if (image)
      {
         auto icon = types::MarkerIconInfo(name, -1, -1, image);
         p->markerIcons_.emplace(name, icon);
      }
      else
      {
         // defaultIconName should always be in markerIcons, so at is fine
         auto icon = p->markerIcons_.at(defaultIconName);
         p->markerIcons_.emplace(name, icon);
      }
   }

   if (!startup)
   {
      ResourceManager::BuildAtlas();
      Q_EMIT IconAdded(name);
   }
}

std::optional<types::MarkerIconInfo>
MarkerManager::get_icon(const std::string& name)
{
   const std::shared_lock lock(p->markerIconsLock_);
   auto                   it = p->markerIcons_.find(name);
   if (it != p->markerIcons_.end())
   {
      return it->second;
   }

   return {};
}

const std::unordered_map<std::string, types::MarkerIconInfo>
MarkerManager::get_icons()
{
   const std::shared_lock lock(p->markerIconsLock_);
   return p->markerIcons_;
}

// Only use for testing
void MarkerManager::set_marker_settings_path(const std::string& path)
{
   p->markerSettingsPath_ = path;
}


std::shared_ptr<MarkerManager> MarkerManager::Instance()
{
   static std::weak_ptr<MarkerManager> markerManagerReference_ {};
   static std::mutex                   instanceMutex_ {};

   const std::unique_lock lock(instanceMutex_);

   std::shared_ptr<MarkerManager> markerManager =
      markerManagerReference_.lock();

   if (markerManager == nullptr)
   {
      markerManager           = std::make_shared<MarkerManager>();
      markerManagerReference_ = markerManager;
   }

   return markerManager;
}

const std::string& MarkerManager::getDefaultIconName()
{
   return defaultIconName;
}

} // namespace manager
} // namespace qt
} // namespace scwx
