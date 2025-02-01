#include <scwx/qt/settings/hotkey_settings.hpp>

#include <QKeySequence>

namespace scwx::qt::settings
{

static const std::string logPrefix_ = "scwx::qt::settings::hotkey_settings";

static const std::unordered_map<types::Hotkey, QKeySequence> kDefaultHotkeys_ {
   {types::Hotkey::AddLocationMarker, QKeySequence {Qt::Key::Key_M}},
   {types::Hotkey::ChangeMapStyle, QKeySequence {Qt::Key::Key_Z}},
   {types::Hotkey::CopyCursorCoordinates,
    QKeySequence {QKeyCombination {Qt::KeyboardModifier::ControlModifier,
                                   Qt::Key::Key_C}}},
   {types::Hotkey::CopyMapCoordinates,
    QKeySequence {QKeyCombination {Qt::KeyboardModifier::ControlModifier |
                                      Qt::KeyboardModifier::ShiftModifier,
                                   Qt::Key::Key_C}}},
   {types::Hotkey::MapPanUp, QKeySequence {Qt::Key::Key_W}},
   {types::Hotkey::MapPanDown, QKeySequence {Qt::Key::Key_S}},
   {types::Hotkey::MapPanLeft, QKeySequence {Qt::Key::Key_A}},
   {types::Hotkey::MapPanRight, QKeySequence {Qt::Key::Key_D}},
   {types::Hotkey::MapRotateClockwise, QKeySequence {Qt::Key::Key_E}},
   {types::Hotkey::MapRotateCounterclockwise, QKeySequence {Qt::Key::Key_Q}},
   {types::Hotkey::MapZoomIn, QKeySequence {Qt::Key::Key_Equal}},
   {types::Hotkey::MapZoomOut, QKeySequence {Qt::Key::Key_Minus}},
   {types::Hotkey::ProductTiltDecrease,
    QKeySequence {Qt::Key::Key_BracketLeft}},
   {types::Hotkey::ProductTiltIncrease,
    QKeySequence {Qt::Key::Key_BracketRight}},
   {types::Hotkey::SelectLevel2Ref, QKeySequence {Qt::Key::Key_F1}},
   {types::Hotkey::SelectLevel2Vel, QKeySequence {Qt::Key::Key_F2}},
   {types::Hotkey::SelectLevel2SW, QKeySequence {Qt::Key::Key_F3}},
   {types::Hotkey::SelectLevel2ZDR, QKeySequence {Qt::Key::Key_F4}},
   {types::Hotkey::SelectLevel2Phi, QKeySequence {Qt::Key::Key_F5}},
   {types::Hotkey::SelectLevel2Rho, QKeySequence {Qt::Key::Key_F6}},
   {types::Hotkey::SelectLevel2CFP, QKeySequence {Qt::Key::Key_F7}},
   {types::Hotkey::SelectLevel3Ref, QKeySequence {Qt::Key::Key_QuoteLeft}},
   {types::Hotkey::SelectLevel3Vel, QKeySequence {Qt::Key::Key_1}},
   {types::Hotkey::SelectLevel3SRM, QKeySequence {Qt::Key::Key_2}},
   {types::Hotkey::SelectLevel3SW, QKeySequence {Qt::Key::Key_3}},
   {types::Hotkey::SelectLevel3ZDR, QKeySequence {Qt::Key::Key_4}},
   {types::Hotkey::SelectLevel3KDP, QKeySequence {Qt::Key::Key_5}},
   {types::Hotkey::SelectLevel3CC, QKeySequence {Qt::Key::Key_6}},
   {types::Hotkey::SelectLevel3VIL, QKeySequence {Qt::Key::Key_7}},
   {types::Hotkey::SelectLevel3ET, QKeySequence {Qt::Key::Key_8}},
   {types::Hotkey::SelectLevel3HC, QKeySequence {Qt::Key::Key_9}},
   {types::Hotkey::SelectLevel3Acc, QKeySequence {Qt::Key::Key_0}},
   {types::Hotkey::TimelineStepBegin,
    QKeySequence {QKeyCombination {Qt::KeyboardModifier::ControlModifier,
                                   Qt::Key::Key_Left}}},
   {types::Hotkey::TimelineStepBack, QKeySequence {Qt::Key::Key_Left}},
   {types::Hotkey::TimelinePlay, QKeySequence {Qt::Key::Key_Space}},
   {types::Hotkey::TimelineStepNext, QKeySequence {Qt::Key::Key_Right}},
   {types::Hotkey::TimelineStepEnd,
    QKeySequence {QKeyCombination {Qt::KeyboardModifier::ControlModifier,
                                   Qt::Key::Key_Right}}},
   {types::Hotkey::Unknown, QKeySequence {}}};

static bool IsHotkeyValid(const std::string& value);

class HotkeySettings::Impl
{
public:
   explicit Impl()
   {
      hotkey_.reserve(types::HotkeyIterator().count() + 1);

      for (const auto& hotkey : types::HotkeyIterator())
      {
         const std::string& name = types::GetHotkeyShortName(hotkey);
         const std::string  defaultValue =
            kDefaultHotkeys_.at(hotkey).toString().toStdString();

         auto result =
            hotkey_.emplace(hotkey, SettingsVariable<std::string> {name});

         SettingsVariable<std::string>& settingsVariable = result.first->second;

         settingsVariable.SetDefault(defaultValue);
         settingsVariable.SetValidator(&IsHotkeyValid);

         variables_.push_back(&settingsVariable);
      }

      // Add an empty hotkey (not part of registered variables) for error
      // handling
      hotkey_.emplace(types::Hotkey::Unknown,
                      SettingsVariable<std::string> {"?"});
   }

   ~Impl()                       = default;
   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   std::unordered_map<types::Hotkey, SettingsVariable<std::string>> hotkey_ {};
   std::vector<SettingsVariableBase*> variables_ {};
};

HotkeySettings::HotkeySettings() :
    SettingsCategory("hotkeys"), p(std::make_unique<Impl>())
{
   RegisterVariables(p->variables_);
   SetDefaults();

   p->variables_.clear();
}
HotkeySettings::~HotkeySettings() = default;

HotkeySettings::HotkeySettings(HotkeySettings&&) noexcept            = default;
HotkeySettings& HotkeySettings::operator=(HotkeySettings&&) noexcept = default;

SettingsVariable<std::string>&
HotkeySettings::hotkey(scwx::qt::types::Hotkey hotkey) const
{
   auto hotkeyVariable = p->hotkey_.find(hotkey);
   if (hotkeyVariable == p->hotkey_.cend())
   {
      hotkeyVariable = p->hotkey_.find(types::Hotkey::Unknown);
   }
   return hotkeyVariable->second;
}

HotkeySettings& HotkeySettings::Instance()
{
   static HotkeySettings hotkeySettings_;
   return hotkeySettings_;
}

bool operator==(const HotkeySettings& lhs, const HotkeySettings& rhs)
{
   return (lhs.p->hotkey_ == rhs.p->hotkey_);
}

static bool IsHotkeyValid(const std::string& value)
{
   return QKeySequence::fromString(QString::fromStdString(value))
             .toString()
             .toStdString() == value;
}

} // namespace scwx::qt::settings
