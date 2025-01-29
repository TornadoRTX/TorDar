#pragma once

#include <scwx/qt/settings/settings_category.hpp>
#include <scwx/qt/settings/settings_variable.hpp>

#include <memory>
#include <string>

#include <boost/gil/typedefs.hpp>

namespace scwx::qt::settings
{

class LineSettings : public SettingsCategory
{
public:
   explicit LineSettings(const std::string& name);
   ~LineSettings() override;

   LineSettings(const LineSettings&)            = delete;
   LineSettings& operator=(const LineSettings&) = delete;

   LineSettings(LineSettings&&) noexcept;
   LineSettings& operator=(LineSettings&&) noexcept;

   [[nodiscard]] SettingsVariable<std::string>& border_color() const;
   [[nodiscard]] SettingsVariable<std::string>& highlight_color() const;
   [[nodiscard]] SettingsVariable<std::string>& line_color() const;

   [[nodiscard]] SettingsVariable<std::int64_t>& border_width() const;
   [[nodiscard]] SettingsVariable<std::int64_t>& highlight_width() const;
   [[nodiscard]] SettingsVariable<std::int64_t>& line_width() const;

   [[nodiscard]] boost::gil::rgba32f_pixel_t GetBorderColorRgba32f() const;
   [[nodiscard]] boost::gil::rgba32f_pixel_t GetHighlightColorRgba32f() const;
   [[nodiscard]] boost::gil::rgba32f_pixel_t GetLineColorRgba32f() const;

   void StageValues(boost::gil::rgba8_pixel_t borderColor,
                    boost::gil::rgba8_pixel_t highlightColor,
                    boost::gil::rgba8_pixel_t lineColor,
                    std::int64_t              borderWidth,
                    std::int64_t              highlightWidth,
                    std::int64_t              lineWidth);

   friend bool operator==(const LineSettings& lhs, const LineSettings& rhs);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::settings
