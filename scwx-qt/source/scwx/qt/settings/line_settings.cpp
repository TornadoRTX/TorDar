#include <scwx/qt/settings/line_settings.hpp>
#include <scwx/qt/util/color.hpp>

namespace scwx::qt::settings
{

static const std::string logPrefix_ = "scwx::qt::settings::line_settings";

static const boost::gil::rgba8_pixel_t kTransparentColor_ {0, 0, 0, 0};
static const std::string               kTransparentColorString_ {
   util::color::ToArgbString(kTransparentColor_)};

static const boost::gil::rgba8_pixel_t kBlackColor_ {0, 0, 0, 255};
static const std::string               kBlackColorString_ {
   util::color::ToArgbString(kBlackColor_)};

static const boost::gil::rgba8_pixel_t kWhiteColor_ {255, 255, 255, 255};
static const std::string               kWhiteColorString_ {
   util::color::ToArgbString(kWhiteColor_)};

class LineSettings::Impl
{
public:
   explicit Impl()
   {
      // SetDefault, SetMinimum, and SetMaximum are descriptive
      // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
      lineColor_.SetDefault(kWhiteColorString_);
      highlightColor_.SetDefault(kTransparentColorString_);
      borderColor_.SetDefault(kBlackColorString_);

      lineWidth_.SetDefault(3);
      highlightWidth_.SetDefault(0);
      borderWidth_.SetDefault(1);

      lineWidth_.SetMinimum(1);
      highlightWidth_.SetMinimum(0);
      borderWidth_.SetMinimum(0);

      lineWidth_.SetMaximum(9);
      highlightWidth_.SetMaximum(9);
      borderWidth_.SetMaximum(9);
      // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

      lineColor_.SetValidator(&util::color::ValidateArgbString);
      highlightColor_.SetValidator(&util::color::ValidateArgbString);
      borderColor_.SetValidator(&util::color::ValidateArgbString);
   }

   ~Impl()                       = default;
   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   SettingsVariable<std::string> lineColor_ {"line_color"};
   SettingsVariable<std::string> highlightColor_ {"highlight_color"};
   SettingsVariable<std::string> borderColor_ {"border_color"};

   SettingsVariable<std::int64_t> lineWidth_ {"line_width"};
   SettingsVariable<std::int64_t> highlightWidth_ {"highlight_width"};
   SettingsVariable<std::int64_t> borderWidth_ {"border_width"};
};

LineSettings::LineSettings(const std::string& name) :
    SettingsCategory(name), p(std::make_unique<Impl>())
{
   RegisterVariables({&p->lineColor_,
                      &p->highlightColor_,
                      &p->borderColor_,
                      &p->lineWidth_,
                      &p->highlightWidth_,
                      &p->borderWidth_});
   SetDefaults();
}
LineSettings::~LineSettings() = default;

LineSettings::LineSettings(LineSettings&&) noexcept            = default;
LineSettings& LineSettings::operator=(LineSettings&&) noexcept = default;

SettingsVariable<std::string>& LineSettings::border_color() const
{
   return p->borderColor_;
}

SettingsVariable<std::string>& LineSettings::highlight_color() const
{
   return p->highlightColor_;
}

SettingsVariable<std::string>& LineSettings::line_color() const
{
   return p->lineColor_;
}

SettingsVariable<std::int64_t>& LineSettings::border_width() const
{
   return p->borderWidth_;
}

SettingsVariable<std::int64_t>& LineSettings::highlight_width() const
{
   return p->highlightWidth_;
}

SettingsVariable<std::int64_t>& LineSettings::line_width() const
{
   return p->lineWidth_;
}

boost::gil::rgba32f_pixel_t LineSettings::GetBorderColorRgba32f() const
{
   return util::color::ToRgba32fPixelT(p->borderColor_.GetValue());
}

boost::gil::rgba32f_pixel_t LineSettings::GetHighlightColorRgba32f() const
{
   return util::color::ToRgba32fPixelT(p->highlightColor_.GetValue());
}

boost::gil::rgba32f_pixel_t LineSettings::GetLineColorRgba32f() const
{
   return util::color::ToRgba32fPixelT(p->lineColor_.GetValue());
}

void LineSettings::StageValues(boost::gil::rgba8_pixel_t borderColor,
                               boost::gil::rgba8_pixel_t highlightColor,
                               boost::gil::rgba8_pixel_t lineColor,
                               std::int64_t              borderWidth,
                               std::int64_t              highlightWidth,
                               std::int64_t              lineWidth)
{
   set_block_signals(true);

   p->borderColor_.StageValue(util::color::ToArgbString(borderColor));
   p->highlightColor_.StageValue(util::color::ToArgbString(highlightColor));
   p->lineColor_.StageValue(util::color::ToArgbString(lineColor));
   p->borderWidth_.StageValue(borderWidth);
   p->highlightWidth_.StageValue(highlightWidth);
   p->lineWidth_.StageValue(lineWidth);

   set_block_signals(false);

   staged_signal()();
}

bool operator==(const LineSettings& lhs, const LineSettings& rhs)
{
   return (lhs.p->borderColor_ == rhs.p->borderColor_ &&
           lhs.p->highlightColor_ == rhs.p->highlightColor_ &&
           lhs.p->lineColor_ == rhs.p->lineColor_ &&
           lhs.p->borderWidth_ == rhs.p->borderWidth_ &&
           lhs.p->highlightWidth_ == rhs.p->highlightWidth_ &&
           lhs.p->lineWidth_ == rhs.p->lineWidth_);
}

} // namespace scwx::qt::settings
