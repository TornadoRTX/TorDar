#include <scwx/qt/types/settings_types.hpp>
#include <scwx/qt/manager/marker_manager.hpp>
#include <scwx/qt/manager/placefile_manager.hpp>
#include <scwx/qt/manager/settings_manager.hpp>
#include <scwx/qt/model/layer_model.hpp>
#include <scwx/qt/model/radar_site_model.hpp>

#include <map>

namespace scwx::qt::types
{

static const std::map<SettingsType, std::string> kSettingsName_ {
   {SettingsType::Settings, "Settings"},
   {SettingsType::Layers, "Layers"},
   {SettingsType::LocationMarkers, "Location Markers"},
   {SettingsType::Placefiles, "Placefiles"},
   {SettingsType::RadarSitePresets, "Radar Site Presets"}};

static const std::map<SettingsType, std::string> kSettingsFilename_ {
   {SettingsType::Settings, "settings.json"},
   {SettingsType::Layers, "layers.json"},
   {SettingsType::LocationMarkers, "location-markers.json"},
   {SettingsType::Placefiles, "placefiles.json"},
   {SettingsType::RadarSitePresets, "radar-presets.json"}};

const std::string& SettingsName(SettingsType type)
{
   return kSettingsName_.at(type);
}

const std::string& SettingsFilename(SettingsType type)
{
   return kSettingsFilename_.at(type);
}

void ReadSettingsFile(SettingsType type, std::istream& is)
{
   switch (type)
   {
   case SettingsType::Settings:
      manager::SettingsManager::Instance().ReadSettings(is);
      break;

   case SettingsType::Layers:
      model::LayerModel::Instance()->ReadLayerSettings(is);
      break;

   case SettingsType::LocationMarkers:
      manager::MarkerManager::Instance()->ReadMarkerSettings(is);
      break;

   case SettingsType::Placefiles:
      manager::PlacefileManager::Instance()->ReadPlacefileSettings(is);
      break;

   case SettingsType::RadarSitePresets:
      model::RadarSiteModel::Instance()->ReadPresets(is);
      break;
   }
}

void WriteSettingsFile(SettingsType type, std::ostream& os)
{
   switch (type)
   {
   case SettingsType::Settings:
      manager::SettingsManager::Instance().WriteSettings(os);
      break;

   case SettingsType::Layers:
      model::LayerModel::Instance()->WriteLayerSettings(os);
      break;

   case SettingsType::LocationMarkers:
      manager::MarkerManager::Instance()->WriteMarkerSettings(os);
      break;

   case SettingsType::Placefiles:
      manager::PlacefileManager::Instance()->WritePlacefileSettings(os);
      break;

   case SettingsType::RadarSitePresets:
      model::RadarSiteModel::Instance()->WritePresets(os);
      break;
   }
}

} // namespace scwx::qt::types
