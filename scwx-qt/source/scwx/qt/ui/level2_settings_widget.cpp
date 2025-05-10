#include <qlabel.h>
#include <scwx/qt/ui/level2_settings_widget.hpp>
#include <scwx/qt/ui/flow_layout.hpp>
#include <scwx/qt/manager/hotkey_manager.hpp>
#include <scwx/common/characters.hpp>
#include <scwx/util/logger.hpp>

#include <execution>

#include <QCheckBox>
#include <QEvent>
#include <QGroupBox>
#include <QToolButton>

namespace scwx
{
namespace qt
{
namespace ui
{

static const std::string logPrefix_ = "scwx::qt::ui::level2_settings_widget";
static const auto        logger_    = util::Logger::Create(logPrefix_);

class Level2SettingsWidgetImpl : public QObject
{
   Q_OBJECT

public:
   explicit Level2SettingsWidgetImpl(Level2SettingsWidget* self) :
       self_ {self},
       layout_ {new QVBoxLayout(self)},
       elevationGroupBox_ {},
       incomingElevationLabel_ {},
       elevationButtons_ {},
       elevationCuts_ {},
       elevationButtonsChanged_ {false},
       resizeElevationButtons_ {false},
       settingsGroupBox_ {},
       declutterCheckBox_ {}
   {
      layout_->setContentsMargins(0, 0, 0, 0);

      incomingElevationLabel_ = new QLabel("", self);
      layout_->addWidget(incomingElevationLabel_);

      elevationGroupBox_ = new QGroupBox(tr("Elevation"), self);
      new ui::FlowLayout(elevationGroupBox_);
      layout_->addWidget(elevationGroupBox_);


      settingsGroupBox_       = new QGroupBox(tr("Settings"), self);
      QLayout* settingsLayout = new QVBoxLayout(settingsGroupBox_);
      layout_->addWidget(settingsGroupBox_);

      declutterCheckBox_ = new QCheckBox(tr("Declutter"), settingsGroupBox_);
      settingsLayout->addWidget(declutterCheckBox_);

      settingsGroupBox_->setVisible(false);

      QObject::connect(hotkeyManager_.get(),
                       &manager::HotkeyManager::HotkeyPressed,
                       this,
                       &Level2SettingsWidgetImpl::HandleHotkeyPressed);
   }
   ~Level2SettingsWidgetImpl() = default;

   void HandleHotkeyPressed(types::Hotkey hotkey, bool isAutoRepeat);
   void NormalizeElevationButtons();
   void SelectElevation(float elevation);

   Level2SettingsWidget* self_;
   QLayout*              layout_;

   QGroupBox*              elevationGroupBox_;
   QLabel*                 incomingElevationLabel_;
   std::list<QToolButton*> elevationButtons_;
   std::vector<float>      elevationCuts_;
   bool                    elevationButtonsChanged_;
   bool                    resizeElevationButtons_;

   QGroupBox* settingsGroupBox_;
   QCheckBox* declutterCheckBox_;

   float        currentElevation_ {};
   QToolButton* currentElevationButton_ {nullptr};

   std::shared_ptr<manager::HotkeyManager> hotkeyManager_ {
      manager::HotkeyManager::Instance()};
};

Level2SettingsWidget::Level2SettingsWidget(QWidget* parent) :
    QWidget(parent), p {std::make_shared<Level2SettingsWidgetImpl>(this)}
{
}

Level2SettingsWidget::~Level2SettingsWidget() {}

bool Level2SettingsWidget::event(QEvent* event)
{
   if (event->type() == QEvent::Type::Paint)
   {
      if (p->elevationButtonsChanged_)
      {
         p->elevationButtonsChanged_ = false;
      }
      else if (p->resizeElevationButtons_)
      {
         p->NormalizeElevationButtons();
      }
   }

   return QWidget::event(event);
}

void Level2SettingsWidget::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);

   p->NormalizeElevationButtons();
}

void Level2SettingsWidgetImpl::HandleHotkeyPressed(types::Hotkey hotkey,
                                                   bool          isAutoRepeat)
{
   if (hotkey != types::Hotkey::ProductTiltDecrease &&
       hotkey != types::Hotkey::ProductTiltIncrease)
   {
      // Not handling this hotkey
      return;
   }

   logger_->trace("Handling hotkey: {}, repeat: {}",
                  types::GetHotkeyShortName(hotkey),
                  isAutoRepeat);

   if (!self_->isVisible() || currentElevationButton_ == nullptr)
   {
      // Level 2 product is not selected
      return;
   }

   // Find the current elevation tilt
   auto tiltIt = std::find(elevationButtons_.cbegin(),
                           elevationButtons_.cend(),
                           currentElevationButton_);

   if (tiltIt == elevationButtons_.cend())
   {
      logger_->error("Could not locate level 2 tilt: {}", currentElevation_);
      return;
   }

   if (hotkey == types::Hotkey::ProductTiltDecrease)
   {
      // Validate the current elevation tilt
      if (tiltIt != elevationButtons_.cbegin())
      {
         // Get the previous elevation tilt
         --tiltIt;

         // Select the new elevation tilt
         (*tiltIt)->click();
      }
      else
      {
         logger_->info("Level 2 tilt at lower limit");
      }
   }
   else if (hotkey == types::Hotkey::ProductTiltIncrease)
   {
      // Get the next elevation tilt
      ++tiltIt;

      // Validate the next elevation tilt
      if (tiltIt != elevationButtons_.cend())
      {
         // Select the new elevation tilt
         (*tiltIt)->click();
      }
      else
      {
         logger_->info("Level 2 tilt at upper limit");
      }
   }
}

void Level2SettingsWidgetImpl::NormalizeElevationButtons()
{
   // Set each elevation cut's tool button to the same size
   int elevationCutMaxWidth = 0;
   std::for_each(elevationButtons_.cbegin(),
                 elevationButtons_.cend(),
                 [&](auto& toolButton)
                 {
                    if (toolButton->isVisible())
                    {
                       elevationCutMaxWidth =
                          std::max(elevationCutMaxWidth, toolButton->width());
                    }
                 });

   // Don't resize the buttons if the size is out of expected ranges
   if (0 < elevationCutMaxWidth && elevationCutMaxWidth < 100)
   {
      std::for_each(elevationButtons_.cbegin(),
                    elevationButtons_.cend(),
                    [&](auto& toolButton)
                    { toolButton->setMinimumWidth(elevationCutMaxWidth); });

      resizeElevationButtons_ = false;
   }
}

void Level2SettingsWidgetImpl::SelectElevation(float elevation)
{
   self_->UpdateElevationSelection(elevation);

   Q_EMIT self_->ElevationSelected(elevation);
}

void Level2SettingsWidget::UpdateElevationSelection(float elevation)
{
   QString buttonText {QString::number(elevation, 'f', 1) +
                       common::Characters::DEGREE};

   QToolButton* newElevationButton = nullptr;

   std::for_each(p->elevationButtons_.cbegin(),
                 p->elevationButtons_.cend(),
                 [&](auto& toolButton)
                 {
                    if (toolButton->text() == buttonText)
                    {
                       newElevationButton = toolButton;
                       toolButton->setCheckable(true);
                       toolButton->setChecked(true);
                    }
                    else
                    {
                       toolButton->setChecked(false);
                       toolButton->setCheckable(false);
                    }
                 });

   p->currentElevation_       = elevation;
   p->currentElevationButton_ = newElevationButton;
}

void Level2SettingsWidget::UpdateIncomingElevation(float incomingElevation)
{
   p->incomingElevationLabel_->setText("Incoming Elevation: " +
      QString::number(incomingElevation, 'f', 1) + common::Characters::DEGREE);
}

void Level2SettingsWidget::UpdateSettings(map::MapWidget* activeMap)
{
   std::optional<float> currentElevationOption = activeMap->GetElevation();
   const float          currentElevation =
      currentElevationOption.has_value() ? *currentElevationOption : 0.0f;
   std::vector<float> elevationCuts    = activeMap->GetElevationCuts();
   float incomingElevation = activeMap->GetIncomingLevel2Elevation();

   if (p->elevationCuts_ != elevationCuts)
   {
      for (auto it = p->elevationButtons_.begin();
           it != p->elevationButtons_.end();)
      {
         delete *it;
         it = p->elevationButtons_.erase(it);
      }

      QLayout* layout = p->elevationGroupBox_->layout();

      // Create elevation cut tool buttons
      for (float elevationCut : elevationCuts)
      {
         QToolButton* toolButton = new QToolButton();
         toolButton->setText(QString::number(elevationCut, 'f', 1) +
                             common::Characters::DEGREE);
         layout->addWidget(toolButton);
         p->elevationButtons_.push_back(toolButton);

         connect(toolButton,
                 &QToolButton::clicked,
                 this,
                 [=, this]() { p->SelectElevation(elevationCut); });
      }

      p->elevationCuts_           = elevationCuts;
      p->elevationButtonsChanged_ = true;
      p->resizeElevationButtons_  = true;
   }

   UpdateElevationSelection(currentElevation);
   UpdateIncomingElevation(incomingElevation);
}

} // namespace ui
} // namespace qt
} // namespace scwx

#include "level2_settings_widget.moc"
