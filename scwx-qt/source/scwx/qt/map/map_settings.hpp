#pragma once

namespace scwx::qt::map
{

struct MapSettings
{
   explicit MapSettings() = default;
   ~MapSettings()         = default;

   MapSettings(const MapSettings&)            = delete;
   MapSettings& operator=(const MapSettings&) = delete;

   MapSettings(MapSettings&&) noexcept            = default;
   MapSettings& operator=(MapSettings&&) noexcept = default;

   bool isActive_ {false};
   bool radarWireframeEnabled_ {false};
};

} // namespace scwx::qt::map
