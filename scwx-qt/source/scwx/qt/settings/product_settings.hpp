#pragma once

#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/qt/settings/settings_variable.hpp>

#include <memory>

namespace scwx::qt::settings
{

class ProductSettings : public SettingsCategory
{
public:
   explicit ProductSettings();
   ~ProductSettings() override;

   ProductSettings(const ProductSettings&)            = delete;
   ProductSettings& operator=(const ProductSettings&) = delete;

   ProductSettings(ProductSettings&&) noexcept;
   ProductSettings& operator=(ProductSettings&&) noexcept;

   SettingsVariable<bool>& show_smoothed_range_folding();
   SettingsVariable<bool>& sti_forecast_enabled();
   SettingsVariable<bool>& sti_past_enabled();

   static ProductSettings& Instance();

   friend bool operator==(const ProductSettings& lhs,
                          const ProductSettings& rhs);

   bool Shutdown();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::settings
