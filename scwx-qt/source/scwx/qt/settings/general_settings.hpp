#pragma once

#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/qt/settings/settings_container.hpp>

#include <memory>
#include <string>

namespace scwx
{
namespace qt
{
namespace settings
{

class GeneralSettings : public SettingsCategory
{
public:
   explicit GeneralSettings();
   ~GeneralSettings();

   GeneralSettings(const GeneralSettings&)            = delete;
   GeneralSettings& operator=(const GeneralSettings&) = delete;

   GeneralSettings(GeneralSettings&&) noexcept;
   GeneralSettings& operator=(GeneralSettings&&) noexcept;

   SettingsVariable<bool>&        anti_aliasing_enabled() const;
   SettingsVariable<std::string>& clock_format() const;
   SettingsVariable<std::string>& custom_style_draw_layer() const;
   SettingsVariable<std::string>& custom_style_url() const;
   SettingsVariable<bool>&        debug_enabled() const;
   SettingsVariable<std::string>& default_alert_action() const;
   SettingsVariable<std::string>& default_radar_site() const;
   SettingsVariable<std::string>& default_time_zone() const;
   SettingsContainer<std::vector<std::int64_t>>& font_sizes() const;
   SettingsVariable<std::int64_t>&               grid_height() const;
   SettingsVariable<std::int64_t>&               grid_width() const;
   SettingsVariable<std::int64_t>&               loop_delay() const;
   SettingsVariable<double>&                     loop_speed() const;
   SettingsVariable<std::int64_t>&               loop_time() const;
   SettingsVariable<std::string>&                map_provider() const;
   SettingsVariable<std::string>&                mapbox_api_key() const;
   SettingsVariable<std::string>&                maptiler_api_key() const;
   SettingsVariable<std::int64_t>&               nmea_baud_rate() const;
   SettingsVariable<std::string>&                nmea_source() const;
   SettingsVariable<std::string>&                positioning_plugin() const;
   SettingsVariable<bool>&        process_module_warnings_enabled() const;
   SettingsVariable<bool>&        show_map_attribution() const;
   SettingsVariable<bool>&        show_map_center() const;
   SettingsVariable<bool>&        show_map_logo() const;
   SettingsVariable<std::string>& theme() const;
   SettingsVariable<std::string>& theme_file() const;
   SettingsVariable<bool>&        track_location() const;
   SettingsVariable<bool>&        update_notifications_enabled() const;
   SettingsVariable<std::string>& warnings_provider() const;
   SettingsVariable<bool>&        cursor_icon_always_on() const;
   SettingsVariable<double>&      radar_site_threshold() const;
   SettingsVariable<bool>&        high_privilege_warning_enabled() const;

   static GeneralSettings& Instance();

   friend bool operator==(const GeneralSettings& lhs,
                          const GeneralSettings& rhs);

   bool Shutdown();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace settings
} // namespace qt
} // namespace scwx
