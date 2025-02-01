#pragma once

#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/qt/settings/settings_variable.hpp>

#include <memory>
#include <string>

namespace scwx::qt::settings
{

class UnitSettings : public SettingsCategory
{
public:
   explicit UnitSettings();
   ~UnitSettings() override;

   UnitSettings(const UnitSettings&)            = delete;
   UnitSettings& operator=(const UnitSettings&) = delete;

   UnitSettings(UnitSettings&&) noexcept;
   UnitSettings& operator=(UnitSettings&&) noexcept;

   [[nodiscard]] SettingsVariable<std::string>& accumulation_units() const;
   [[nodiscard]] SettingsVariable<std::string>& echo_tops_units() const;
   [[nodiscard]] SettingsVariable<std::string>& other_units() const;
   [[nodiscard]] SettingsVariable<std::string>& speed_units() const;
   [[nodiscard]] SettingsVariable<std::string>& distance_units() const;

   static UnitSettings& Instance();

   friend bool operator==(const UnitSettings& lhs, const UnitSettings& rhs);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::settings
