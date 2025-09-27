#include <scwx/qt/manager/alert_manager.hpp>
#include <scwx/qt/manager/media_manager.hpp>
#include <scwx/qt/manager/position_manager.hpp>
#include <scwx/qt/manager/text_event_manager.hpp>
#include <scwx/qt/config/radar_site.hpp>
#include <scwx/qt/settings/audio_settings.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/types/location_types.hpp>
#include <scwx/qt/util/geographic_lib.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/time.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/uuid/random_generator.hpp>
#include <QGeoPositionInfo>

namespace scwx
{
namespace qt
{
namespace manager
{

static const std::string logPrefix_ = "scwx::qt::manager::alert_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class AlertManager::Impl
{
public:
   explicit Impl(AlertManager* self) : self_ {self}
   {
      settings::AudioSettings& audioSettings =
         settings::AudioSettings::Instance();

      UpdateLocationTracking(audioSettings.alert_location_method().GetValue());

      audioSettings.alert_location_method().RegisterValueChangedCallback(
         [this](const std::string& value) { UpdateLocationTracking(value); });

      QObject::connect(
         textEventManager_.get(),
         &manager::TextEventManager::AlertUpdated,
         self_,
         [this](const types::TextEventKey& key, size_t messageIndex)
         {
            boost::asio::post(threadPool_,
                              [=, this]()
                              {
                                 try
                                 {
                                    HandleAlert(key, messageIndex);
                                 }
                                 catch (const std::exception& ex)
                                 {
                                    logger_->error(ex.what());
                                 }
                              });
         });
   }

   ~Impl() { threadPool_.join(); }

   common::Coordinate
        CurrentCoordinate(types::LocationMethod locationMethod) const;
   void HandleAlert(const types::TextEventKey& key, size_t messageIndex) const;
   void UpdateLocationTracking(const std::string& value) const;

   boost::asio::thread_pool threadPool_ {1u};

   AlertManager* self_;

   boost::uuids::uuid uuid_ {boost::uuids::random_generator()()};

   std::shared_ptr<MediaManager>    mediaManager_ {MediaManager::Instance()};
   std::shared_ptr<PositionManager> positionManager_ {
      PositionManager::Instance()};
   std::shared_ptr<TextEventManager> textEventManager_ {
      TextEventManager::Instance()};

   std::shared_ptr<config::RadarSite> radarSite_ {};
};

AlertManager::AlertManager() : p(std::make_unique<Impl>(this)) {}
AlertManager::~AlertManager() = default;

common::Coordinate AlertManager::Impl::CurrentCoordinate(
   types::LocationMethod locationMethod) const
{
   settings::AudioSettings& audioSettings = settings::AudioSettings::Instance();
   common::Coordinate       coordinate {};

   if (locationMethod == types::LocationMethod::Fixed)
   {
      coordinate.latitude_  = audioSettings.alert_latitude().GetValue();
      coordinate.longitude_ = audioSettings.alert_longitude().GetValue();
   }
   else if (locationMethod == types::LocationMethod::Track)
   {
      QGeoPositionInfo position = positionManager_->position();
      if (position.isValid())
      {
         QGeoCoordinate trackedCoordinate = position.coordinate();
         coordinate.latitude_             = trackedCoordinate.latitude();
         coordinate.longitude_            = trackedCoordinate.longitude();
      }
   }
   else if (locationMethod == types::LocationMethod::RadarSite)
   {
      std::string radarSiteSelection =
         audioSettings.alert_radar_site().GetValue();
      std::shared_ptr<config::RadarSite> radarSite;
      if (radarSiteSelection == "default")
      {
         std::string siteId = settings::GeneralSettings::Instance()
                                 .default_radar_site()
                                 .GetValue();
         radarSite = config::RadarSite::Get(siteId);
      }
      else if (radarSiteSelection == "follow")
      {
         radarSite = radarSite_;
      }
      else
      {
         radarSite = config::RadarSite::Get(radarSiteSelection);
      }

      if (radarSite != nullptr)
      {
         coordinate.latitude_  = radarSite->latitude();
         coordinate.longitude_ = radarSite->longitude();
      }
   }

   return coordinate;
}

void AlertManager::Impl::HandleAlert(const types::TextEventKey& key,
                                     size_t messageIndex) const
{
   auto messages = textEventManager_->message_list(key);

   // Skip alert if there are more messages to be processed
   if (messages.empty() || messageIndex + 1 < messages.size())
   {
      return;
   }

   settings::AudioSettings& audioSettings = settings::AudioSettings::Instance();
   types::LocationMethod    locationMethod = types::GetLocationMethod(
      audioSettings.alert_location_method().GetValue());
   common::Coordinate currentCoordinate = CurrentCoordinate(locationMethod);
   std::string        alertCounty = audioSettings.alert_county().GetValue();
   auto               alertRadius = units::length::kilometers<double>(
      audioSettings.alert_radius().GetValue());
   std::string alertWFO = audioSettings.alert_wfo().GetValue();

   auto message = messages.at(messageIndex);

   for (auto& segment : message->segments())
   {
      if (!segment->codedLocation_.has_value())
      {
         continue;
      }

      auto&             vtec       = segment->header_->vtecString_.front();
      auto              action     = vtec.pVtec_.action();
      awips::Phenomenon phenomenon = vtec.pVtec_.phenomenon();
      auto              eventEnd   = vtec.pVtec_.event_end();
      bool alertActive             = (action != awips::PVtec::Action::Canceled);

      // If the event has ended or is inactive, or if the alert is not enabled,
      // skip it
      if (eventEnd < scwx::util::time::now() || !alertActive ||
          !audioSettings.alert_enabled(phenomenon).GetValue())
      {
         continue;
      }

      bool activeAtLocation = (locationMethod == types::LocationMethod::All);

      if (locationMethod == types::LocationMethod::Fixed ||
          locationMethod == types::LocationMethod::Track ||
          locationMethod == types::LocationMethod::RadarSite)
      {
         // Determine if the alert is active at the current coordinte
         auto alertCoordinates = segment->codedLocation_->coordinates();

         activeAtLocation = util::GeographicLib::AreaInRangeOfPoint(
            alertCoordinates, currentCoordinate, alertRadius);
      }
      else if (locationMethod == types::LocationMethod::County)
      {
         // Determine if the alert contains the current county
         auto fipsIds = segment->header_->ugc_.fips_ids();
         auto it = std::find(fipsIds.cbegin(), fipsIds.cend(), alertCounty);
         activeAtLocation = it != fipsIds.cend();
      }
      else if (locationMethod == types::LocationMethod::WFO)
      {
         std::string wfoId = vtec.pVtec_.office_id();

         activeAtLocation = wfoId == alertWFO;
      }

      if (activeAtLocation)
      {
         logger_->info("Alert active at current location: {} {}.{} {}",
                       vtec.pVtec_.office_id(),
                       awips::GetPhenomenonCode(vtec.pVtec_.phenomenon()),
                       awips::PVtec::GetActionCode(vtec.pVtec_.action()),
                       vtec.pVtec_.event_tracking_number());

         mediaManager_->Play(audioSettings.alert_sound_file().GetValue());
      }
   }
}

void AlertManager::Impl::UpdateLocationTracking(
   const std::string& locationMethodName) const
{
   types::LocationMethod locationMethod =
      types::GetLocationMethod(locationMethodName);
   bool locationEnabled = locationMethod == types::LocationMethod::Track;
   positionManager_->EnablePositionUpdates(uuid_, locationEnabled);
}

void AlertManager::SetRadarSite(
   const std::shared_ptr<config::RadarSite>& radarSite)
{
   if (p->radarSite_ == radarSite)
   {
      // No action needed
      return;
   }

   if (radarSite == nullptr)
   {
      logger_->debug("SetRadarSite: ?");
   }
   else
   {
      logger_->debug("SetRadarSite: {}", radarSite->id());
   }

   p->radarSite_ = radarSite;
}

std::shared_ptr<AlertManager> AlertManager::Instance()
{
   static std::weak_ptr<AlertManager> alertManagerReference_ {};
   static std::mutex                  instanceMutex_ {};

   std::unique_lock lock(instanceMutex_);

   std::shared_ptr<AlertManager> alertManager = alertManagerReference_.lock();

   if (alertManager == nullptr)
   {
      alertManager           = std::make_shared<AlertManager>();
      alertManagerReference_ = alertManager;
   }

   return alertManager;
}

} // namespace manager
} // namespace qt
} // namespace scwx
