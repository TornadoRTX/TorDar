#include <scwx/qt/settings/product_settings.hpp>
#include <scwx/qt/settings/settings_container.hpp>

namespace scwx
{
namespace qt
{
namespace settings
{

static const std::string logPrefix_ = "scwx::qt::settings::product_settings";

class ProductSettings::Impl
{
public:
   explicit Impl()
   {
      showSmoothedRangeFolding_.SetDefault(false);
      stiForecastEnabled_.SetDefault(true);
      stiPastEnabled_.SetDefault(true);
   }

   ~Impl() {}

   SettingsVariable<bool> showSmoothedRangeFolding_ {
      "show_smoothed_range_folding"};
   SettingsVariable<bool> stiForecastEnabled_ {"sti_forecast_enabled"};
   SettingsVariable<bool> stiPastEnabled_ {"sti_past_enabled"};
};

ProductSettings::ProductSettings() :
    SettingsCategory("product"), p(std::make_unique<Impl>())
{
   RegisterVariables({&p->showSmoothedRangeFolding_,
                      &p->stiForecastEnabled_,
                      &p->stiPastEnabled_});
   SetDefaults();
}
ProductSettings::~ProductSettings() = default;

ProductSettings::ProductSettings(ProductSettings&&) noexcept = default;
ProductSettings&
ProductSettings::operator=(ProductSettings&&) noexcept = default;

SettingsVariable<bool>& ProductSettings::show_smoothed_range_folding()
{
   return p->showSmoothedRangeFolding_;
}

SettingsVariable<bool>& ProductSettings::sti_forecast_enabled()
{
   return p->stiForecastEnabled_;
}

SettingsVariable<bool>& ProductSettings::sti_past_enabled()
{
   return p->stiPastEnabled_;
}

bool ProductSettings::Shutdown()
{
   bool dataChanged = false;

   // Commit settings that are managed separate from the settings dialog
   dataChanged |= p->stiForecastEnabled_.Commit();
   dataChanged |= p->stiPastEnabled_.Commit();

   return dataChanged;
}

ProductSettings& ProductSettings::Instance()
{
   static ProductSettings generalSettings_;
   return generalSettings_;
}

bool operator==(const ProductSettings& lhs, const ProductSettings& rhs)
{
   return (lhs.p->showSmoothedRangeFolding_ ==
              rhs.p->showSmoothedRangeFolding_ &&
           lhs.p->stiForecastEnabled_ == rhs.p->stiForecastEnabled_ &&
           lhs.p->stiPastEnabled_ == rhs.p->stiPastEnabled_);
}

} // namespace settings
} // namespace qt
} // namespace scwx
