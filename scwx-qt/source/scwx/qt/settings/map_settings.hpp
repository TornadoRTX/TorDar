#pragma once

#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/qt/settings/settings_variable.hpp>

#include <memory>
#include <string>

namespace scwx::qt::settings
{

class MapSettings : public SettingsCategory
{
public:
   explicit MapSettings();
   ~MapSettings() override;

   MapSettings(const MapSettings&)            = delete;
   MapSettings& operator=(const MapSettings&) = delete;

   MapSettings(MapSettings&&) noexcept;
   MapSettings& operator=(MapSettings&&) noexcept;

   [[nodiscard]] std::size_t      count() const;
   SettingsVariable<std::string>& map_style(std::size_t i);
   SettingsVariable<std::string>& radar_site(std::size_t i);
   SettingsVariable<std::string>& radar_product_group(std::size_t i);
   SettingsVariable<std::string>& radar_product(std::size_t i);
   SettingsVariable<bool>&        smoothing_enabled(std::size_t i);

   bool Shutdown();

   /**
    * Reads the variables from the JSON object.
    *
    * @param json JSON object to read
    *
    * @return true if the values read are valid, false if any values were
    * modified.
    */
   bool ReadJson(const boost::json::object& json) override;

   /**
    * Writes the variables to the JSON object.
    *
    * @param json JSON object to write
    */
   void WriteJson(boost::json::object& json) const override;

   static MapSettings& Instance();

   friend bool operator==(const MapSettings& lhs, const MapSettings& rhs);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::settings
