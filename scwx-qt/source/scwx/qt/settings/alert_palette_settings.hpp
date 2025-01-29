#pragma once

#include <scwx/qt/settings/line_settings.hpp>
#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/awips/impact_based_warnings.hpp>
#include <scwx/awips/phenomenon.hpp>

#include <memory>
#include <string>

namespace scwx::qt::settings
{

class AlertPaletteSettings : public SettingsCategory
{
public:
   explicit AlertPaletteSettings(awips::Phenomenon phenomenon);
   ~AlertPaletteSettings() override;

   AlertPaletteSettings(const AlertPaletteSettings&)            = delete;
   AlertPaletteSettings& operator=(const AlertPaletteSettings&) = delete;

   AlertPaletteSettings(AlertPaletteSettings&&) noexcept;
   AlertPaletteSettings& operator=(AlertPaletteSettings&&) noexcept;

   [[nodiscard]] LineSettings&
   threat_category(awips::ibw::ThreatCategory threatCategory) const;
   [[nodiscard]] LineSettings& inactive() const;
   [[nodiscard]] LineSettings& observed() const;
   [[nodiscard]] LineSettings& tornado_possible() const;

   friend bool operator==(const AlertPaletteSettings& lhs,
                          const AlertPaletteSettings& rhs);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::settings
