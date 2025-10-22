#pragma once

#include <scwx/util/iterator.hpp>

#include <cstdint>
#include <string>

namespace scwx::qt::types
{

enum class SettingsType : std::uint8_t
{
   Settings,
   Layers,
   LocationMarkers,
   Placefiles,
   RadarSitePresets
};
using SettingsTypeIterator =
   scwx::util::Iterator<SettingsType,
                        SettingsType::Settings,
                        SettingsType::RadarSitePresets>;

const std::string& SettingsName(SettingsType type);
const std::string& SettingsFilename(SettingsType type);

} // namespace scwx::qt::types
