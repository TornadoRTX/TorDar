#include <scwx/qt/types/settings_types.hpp>

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

} // namespace scwx::qt::types
