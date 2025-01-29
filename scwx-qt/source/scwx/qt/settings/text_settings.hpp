#pragma once

#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/qt/settings/settings_variable.hpp>
#include <scwx/qt/types/text_types.hpp>

#include <memory>
#include <string>

namespace scwx::qt::settings
{

class TextSettings : public SettingsCategory
{
public:
   explicit TextSettings();
   ~TextSettings() override;

   TextSettings(const TextSettings&)            = delete;
   TextSettings& operator=(const TextSettings&) = delete;

   TextSettings(TextSettings&&) noexcept;
   TextSettings& operator=(TextSettings&&) noexcept;

   [[nodiscard]] SettingsVariable<std::string>&
   font_family(types::FontCategory fontCategory) const;
   [[nodiscard]] SettingsVariable<std::string>&
   font_style(types::FontCategory fontCategory) const;
   [[nodiscard]] SettingsVariable<double>&
   font_point_size(types::FontCategory fontCategory) const;

   [[nodiscard]] SettingsVariable<std::int64_t>& hover_text_wrap() const;
   [[nodiscard]] SettingsVariable<bool>&
   placefile_text_drop_shadow_enabled() const;
   [[nodiscard]] SettingsVariable<bool>& radar_site_hover_text_enabled() const;
   [[nodiscard]] SettingsVariable<std::string>& tooltip_method() const;

   static TextSettings& Instance();

   friend bool operator==(const TextSettings& lhs, const TextSettings& rhs);

private:
   class Impl;

   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::settings
