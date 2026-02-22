#include "warning_box_widget.hpp"
#include "ui_warning_box_widget.h"

#include <scwx/awips/phenomenon.hpp>
#include <scwx/awips/significance.hpp>
#include <scwx/qt/config/county_database.hpp>
#include <scwx/qt/manager/text_event_manager.hpp>
#include <scwx/qt/ui/alert_dialog.hpp>
#include <scwx/util/strings.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/time.hpp>

#include <fmt/format.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QLayoutItem>
#include <QMargins>
#include <QScrollBar>

#include <algorithm>
#include <array>
#include <cctype>
#include <initializer_list>
#include <string_view>
#include <unordered_map>

namespace scwx
{
namespace qt
{
namespace ui
{

static const std::string logPrefix_ = "scwx::qt::ui::warning_box_widget";
static const auto        logger_    = util::Logger::Create(logPrefix_);

static std::string LoadFontFamily(const QString& resourcePath,
                                  const char*    fallbackFamily)
{
   const int fontId = QFontDatabase::addApplicationFont(resourcePath);
   if (fontId >= 0)
   {
      const QStringList families =
         QFontDatabase::applicationFontFamilies(fontId);
      if (!families.isEmpty())
      {
         return families.first().toStdString();
      }
   }
   return fallbackFamily;
}

class WarningBoxWidgetImpl : public QObject
{
   Q_OBJECT

public:
   explicit WarningBoxWidgetImpl(WarningBoxWidget* self) :
       self_ {self},
       textEventManager_ {manager::TextEventManager::Instance()},
       alertDialog_ {nullptr},
       updateTimer_ {new QTimer(self)},
       sweepTimer_ {new QTimer(self)},
       currentKey_ {}
   {
      updateTimer_->setSingleShot(false);
      updateTimer_->setInterval(30000);
      QObject::connect(updateTimer_,
                       &QTimer::timeout,
                       this,
                       &WarningBoxWidgetImpl::UpdateCountdown);
      QObject::connect(textEventManager_.get(),
                       &manager::TextEventManager::AlertUpdated,
                       this,
                       [this](const types::TextEventKey& key,
                              std::size_t,
                              boost::uuids::uuid uuid)
                       {
                          if (self_->isVisible() && key == currentKey_ &&
                              uuid != currentMessageUuid_)
                          {
                             PopulateFromWarning(key);
                          }
                       });

      sweepTimer_->setSingleShot(false);
      sweepTimer_->setInterval(33);
      QObject::connect(sweepTimer_,
                       &QTimer::timeout,
                       this,
                       [this]()
                       {
                          if (progressTrack_ == nullptr ||
                              progressSweep_ == nullptr ||
                              !progressTrack_->isVisible())
                          {
                             return;
                          }

                          const int trackWidth = progressTrack_->width();
                          const int sweepWidth = progressSweep_->width();
                          if (trackWidth <= sweepWidth || sweepWidth <= 0)
                          {
                             return;
                          }

                          sweepPosition_ += 3;
                          if (sweepPosition_ > trackWidth)
                          {
                             sweepPosition_ = -sweepWidth;
                          }
                          progressSweep_->move(sweepPosition_, 0);
                       });
   }
   ~WarningBoxWidgetImpl() = default;

   void PopulateFromWarning(const types::TextEventKey& key);
   void UpdateCountdown();
   void UpdateExpirationOnly();
   void
        UpdateProgressVisual(const types::TextEventKey&                   key,
                             const std::shared_ptr<const awips::Segment>& segment);
   void ApplyTheme(const types::TextEventKey&                   key,
                   const std::shared_ptr<const awips::Segment>& segment);
   void UpdateTitleFont();
   void AdjustHeightToContents();
   void AddDetailRow(const std::string& label, const std::string& value);
   void AddSevereMetricCards(const std::string& maxHail,
                             const std::string& maxWind);
   static std::string ToUpper(std::string_view value);
   static std::string Trim(std::string_view value);
   static std::string NormalizeLabel(std::string_view label);
   static bool        IsLabelLine(std::string_view line);
   static std::unordered_map<std::string, std::string>
   ParseProductFields(const std::shared_ptr<const awips::Segment>& segment);
   static std::string
   GetFieldValue(const std::unordered_map<std::string, std::string>& fields,
                 const std::initializer_list<std::string_view>&      keys);
   void
        AddSummaryField(const std::string& summaryLabel,
                        const std::unordered_map<std::string, std::string>& fields,
                        const std::initializer_list<std::string_view>&      keys);
   void AddPhenomenonSpecificFields(
      awips::Phenomenon                                   phenomenon,
      const std::unordered_map<std::string, std::string>& fields);

   WarningBoxWidget*                          self_;
   std::shared_ptr<manager::TextEventManager> textEventManager_;
   AlertDialog*                               alertDialog_;
   QTimer*                                    updateTimer_;
   QTimer*                                    sweepTimer_ {nullptr};
   types::TextEventKey                        currentKey_;
   boost::uuids::uuid                         currentMessageUuid_ {};
   QWidget*                                   stateBadgeFrame_ {nullptr};
   QLabel*                                    stateBadgeLabel_ {nullptr};
   QFrame*                                    progressTrack_ {nullptr};
   QFrame*                                    progressFill_ {nullptr};
   QFrame*                                    progressSweep_ {nullptr};
   int                                        sweepPosition_ {0};
   int                                        fixedWidth_ {0};
   QWidget*                                   detailsContainer_ {nullptr};
   QVBoxLayout*                               detailsLayout_ {nullptr};
   std::string warningTitleFontFamily_ {"Sinter-Bold"};
   std::string expirationFontFamily_ {"IBMPlexMono-SemiBold"};
   std::string monoFontFamily_ {"RobotoMono-Regular"};
   std::string areaSourceValueFontFamily_ {"Sinter-Bold"};
};

WarningBoxWidget::WarningBoxWidget(QWidget* parent) :
    QWidget(parent),
    p {std::make_unique<WarningBoxWidgetImpl>(this)},
    ui(new Ui::WarningBoxWidget)
{
   ui->setupUi(this);

   p->warningTitleFontFamily_ = LoadFontFamily(
      ":/res/fonts/Sinter-Bold", "Sinter-Bold");
   p->expirationFontFamily_ = LoadFontFamily(
      ":/res/fonts/IBMPlexMono-SemiBold.ttf", "IBMPlexMono-SemiBold");
   p->monoFontFamily_ = LoadFontFamily(":/res/fonts/RobotoMono-Regular.ttf",
                                       "RobotoMono-Regular");
   p->areaSourceValueFontFamily_ =
      LoadFontFamily(":/res/fonts/TitilliumWeb-SemiBold",
                     "TitilliumWeb-SemiBold");

   setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground, false);
   setAttribute(Qt::WA_StyledBackground, true);
   setAutoFillBackground(true);

   ui->warningTypeLabel->setTextInteractionFlags(Qt::NoTextInteraction);
   ui->warningTypeLabel->setWordWrap(true);
   ui->warningTypeLabel->setStyleSheet(
      QString("font-family: '%1';"
              "font-size: 20px; font-weight: 800; letter-spacing: 1px;")
         .arg(QString::fromStdString(p->warningTitleFontFamily_)));
   ui->expirationLabel->setStyleSheet(
      QString("font-family: '%1';"
              "font-size: 13px; font-weight: 700; letter-spacing: 0.5px;")
         .arg(QString::fromStdString(p->expirationFontFamily_)));
   ui->viewEasTextButton->setText("VIEW FULL EAS TEXT");
   ui->closeButton->setText("x");
   ui->closeButton->setFixedSize(28, 28);
   ui->buttonsLayout->setContentsMargins(0, 0, 0, 0);
   ui->buttonsLayout->setSpacing(0);
   ui->verticalLayout->setContentsMargins(10, 8, 10, 8);
   ui->verticalLayout->setSpacing(6);
   ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   ui->buttonsLayout->removeWidget(ui->closeButton);
   while (ui->buttonsLayout->count() > 0)
   {
      QLayoutItem* item = ui->buttonsLayout->takeAt(0);
      if (item != nullptr)
      {
         delete item;
      }
   }
   ui->buttonsLayout->addWidget(ui->viewEasTextButton);

   QHBoxLayout* headerLayout = new QHBoxLayout();
   ui->verticalLayout->removeWidget(ui->warningTypeLabel);
   headerLayout->setContentsMargins(0, 0, 0, 0);
   headerLayout->setSpacing(6);
   headerLayout->addWidget(ui->warningTypeLabel, 1);
   headerLayout->addWidget(ui->closeButton, 0, Qt::AlignTop);
   ui->verticalLayout->insertLayout(0, headerLayout);

   p->progressTrack_ = new QFrame(this);
   p->progressTrack_->setObjectName("progressTrack");
   p->progressTrack_->setFixedHeight(6);
   p->progressTrack_->setVisible(false);
   p->progressFill_ = new QFrame(p->progressTrack_);
   p->progressFill_->setObjectName("progressFill");
   p->progressFill_->setGeometry(0, 0, 0, p->progressTrack_->height());
   p->progressSweep_ = new QFrame(p->progressTrack_);
   p->progressSweep_->setObjectName("progressSweep");
   p->progressSweep_->setFixedSize(68, p->progressTrack_->height());
   p->progressSweep_->move(-p->progressSweep_->width(), 0);
   ui->verticalLayout->insertWidget(2, p->progressTrack_);

   p->stateBadgeFrame_ = new QFrame(this);
   p->stateBadgeFrame_->setObjectName("stateBadgeFrame");
   QHBoxLayout* badgeLayout = new QHBoxLayout(p->stateBadgeFrame_);
   badgeLayout->setContentsMargins(8, 4, 8, 4);
   badgeLayout->setSpacing(0);
   p->stateBadgeLabel_ = new QLabel(p->stateBadgeFrame_);
   p->stateBadgeLabel_->setObjectName("stateBadgeLabel");
   badgeLayout->addWidget(p->stateBadgeLabel_);
   p->stateBadgeFrame_->setVisible(false);
   ui->verticalLayout->insertWidget(3, p->stateBadgeFrame_, 0, Qt::AlignLeft);

   hide();

   connect(ui->closeButton,
           &QPushButton::clicked,
           this,
           &WarningBoxWidget::on_closeButton_clicked);
   connect(ui->viewEasTextButton,
           &QPushButton::clicked,
           this,
           &WarningBoxWidget::on_viewEasTextButton_clicked);

   p->detailsContainer_ = new QWidget(this);
   p->detailsLayout_    = new QVBoxLayout(p->detailsContainer_);
   p->detailsLayout_->setContentsMargins(0, 0, 0, 0);
   p->detailsLayout_->setSpacing(4);
   ui->scrollArea->setWidget(p->detailsContainer_);
   ui->scrollArea->setWidgetResizable(true);

   p->fixedWidth_ = width();
   setMinimumWidth(p->fixedWidth_);
   setMaximumWidth(p->fixedWidth_);
}

WarningBoxWidget::~WarningBoxWidget()
{
   delete ui;
}

void WarningBoxWidget::ShowWarning(const types::TextEventKey& key)
{
   p->currentKey_ = key;
   p->PopulateFromWarning(key);
   p->updateTimer_->start();
   show();
   raise();

   // Re-run sizing after the widget is shown so layout metrics are final.
   QTimer::singleShot(0,
                      this,
                      [this]()
                      {
                         if (isVisible())
                         {
                            p->AdjustHeightToContents();
                            p->UpdateTitleFont();
                            p->UpdateExpirationOnly();
                         }
                      });
}

void WarningBoxWidget::HideWarning()
{
   p->updateTimer_->stop();
   p->sweepTimer_->stop();
   hide();
}

void WarningBoxWidget::on_closeButton_clicked()
{
   HideWarning();
}

void WarningBoxWidget::on_viewEasTextButton_clicked()
{
   if (p->alertDialog_ == nullptr)
      p->alertDialog_ = new AlertDialog(this);
   p->alertDialog_->SelectAlert(p->currentKey_);
   p->alertDialog_->show();
}

void WarningBoxWidgetImpl::PopulateFromWarning(const types::TextEventKey& key)
{
   auto messages = textEventManager_->message_list(key);
   if (messages.empty())
      return;

   const auto message  = messages.back();
   currentMessageUuid_ = message->uuid();
   const auto segments = message->segments();
   if (segments.empty())
      return;

   const auto segment = segments.back();
   ApplyTheme(key, segment);

   // Title: from phenomenon and significance (e.g. "Tornado Warning")
   std::string phenText = awips::GetPhenomenonText(key.phenomenon_);
   std::string sigText  = awips::GetSignificanceText(key.significance_);
   std::string title    = ToUpper(fmt::format("{} {}", phenText, sigText));
   if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      if (segment->threatCategory_ == awips::ibw::ThreatCategory::Catastrophic)
      {
         title = "TORNADO EMERGENCY";
      }
      else if (segment->threatCategory_ ==
               awips::ibw::ThreatCategory::Destructive)
      {
         title = "PDS TORNADO WARNING";
      }
   }
   self_->ui->warningTypeLabel->setText(QString::fromStdString(title));

   UpdateExpirationOnly();
   UpdateProgressVisual(key, segment);

   // Affected areas: only if we have UGC data
   std::string countiesStr;
   std::string statesStr;
   if (segment->header_.has_value())
   {
      auto                     fipsIds = segment->header_->ugc_.fips_ids();
      std::vector<std::string> countyNames;
      for (auto& id : fipsIds)
         countyNames.push_back(config::CountyDatabase::GetCountyName(id));
      std::sort(countyNames.begin(), countyNames.end());
      countiesStr = scwx::util::ToString(countyNames);
      statesStr   = scwx::util::ToString(segment->header_->ugc_.states());
   }
   if (!statesStr.empty())
   {
      stateBadgeLabel_->setText(QString::fromStdString(statesStr));
      stateBadgeFrame_->setVisible(true);
   }
   else
   {
      stateBadgeFrame_->setVisible(false);
   }
   self_->ui->areasFrame->setVisible(false);

   // Clear previous detail rows
   while (QLayoutItem* item = detailsLayout_->takeAt(0))
   {
      if (item->widget())
         delete item->widget();
      delete item;
   }

   const auto fields = ParseProductFields(segment);

   if (key.phenomenon_ == awips::Phenomenon::SevereThunderstorm)
   {
      AddSevereMetricCards(GetFieldValue(fields, {"MAX HAIL SIZE"}),
                           GetFieldValue(fields, {"MAX WIND GUST"}));

      if (!countiesStr.empty())
      {
         AddDetailRow("Areas", countiesStr);
      }
      AddSummaryField("Source", fields, {"SOURCE"});
      AddSummaryField(
         "Damage", fields, {"THUNDERSTORM DAMAGE THREAT", "DAMAGE THREAT"});
   }
   else
   {
      if (!countiesStr.empty())
      {
         AddDetailRow("Areas", countiesStr);
      }
      AddSummaryField("Source", fields, {"SOURCE"});
      AddPhenomenonSpecificFields(key.phenomenon_, fields);
   }

   detailsLayout_->addStretch();
   AdjustHeightToContents();
}

void WarningBoxWidgetImpl::AddDetailRow(const std::string& label,
                                        const std::string& value)
{
   QFrame* rowFrame = new QFrame(detailsContainer_);
   rowFrame->setObjectName("detailRow");
   rowFrame->setProperty("highlight", label == "Source");

   QLabel* labelW = new QLabel(QString::fromStdString(ToUpper(label)));
   labelW->setObjectName("detailLabel");

   QLabel* valueW = new QLabel(QString::fromStdString(value));
   valueW->setObjectName("detailValue");
   valueW->setWordWrap(true);

   if (label == "Areas" || label == "Source")
   {
      labelW->setStyleSheet(
         QString("font-family: '%1';")
            .arg(QString::fromStdString(expirationFontFamily_)));
      valueW->setStyleSheet(
         QString("font-family: '%1';")
            .arg(QString::fromStdString(areaSourceValueFontFamily_)));
   }

   QHBoxLayout* row = new QHBoxLayout(rowFrame);
   row->setContentsMargins(8, 6, 8, 6);
   row->setSpacing(6);
   row->addWidget(labelW, 0);
   row->addWidget(valueW, 1);

   detailsLayout_->addWidget(rowFrame);
}

void WarningBoxWidgetImpl::AddSevereMetricCards(const std::string& maxHail,
                                                const std::string& maxWind)
{
   QWidget*     rowContainer = new QWidget(detailsContainer_);
   QHBoxLayout* row          = new QHBoxLayout(rowContainer);
   row->setContentsMargins(0, 0, 0, 0);
   row->setSpacing(8);

   auto createCard =
      [rowContainer](const std::string& title, const std::string& value)
   {
      QFrame* card = new QFrame(rowContainer);
      card->setObjectName("severeMetricCard");
      card->setMinimumHeight(68);
      QVBoxLayout* cardLayout = new QVBoxLayout(card);
      cardLayout->setContentsMargins(8, 6, 8, 6);
      cardLayout->setSpacing(2);

      QLabel* titleLabel =
         new QLabel(QString::fromStdString(ToUpper(title)), card);
      titleLabel->setObjectName("severeMetricTitle");
      QLabel* valueLabel = new QLabel(QString::fromStdString(value), card);
      valueLabel->setObjectName("severeMetricValue");
      auto* valueGlow = new QGraphicsDropShadowEffect(valueLabel);
      valueGlow->setBlurRadius(12.0);
      valueGlow->setColor(QColor(236, 193, 74, 210));
      valueGlow->setOffset(0.0, 0.0);
      valueLabel->setGraphicsEffect(valueGlow);

      cardLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
      cardLayout->addWidget(valueLabel, 0, Qt::AlignHCenter);
      return card;
   };

   if (!maxHail.empty())
   {
      row->addWidget(createCard("Max Hail", maxHail), 1);
   }
   if (!maxWind.empty())
   {
      row->addWidget(createCard("Max Wind", maxWind), 1);
   }

   if (row->count() > 0)
   {
      detailsLayout_->addWidget(rowContainer);
   }
   else
   {
      delete rowContainer;
   }
}

void WarningBoxWidgetImpl::UpdateProgressVisual(
   const types::TextEventKey&                   key,
   const std::shared_ptr<const awips::Segment>& segment)
{
   if (progressTrack_ == nullptr || progressFill_ == nullptr ||
       progressSweep_ == nullptr)
   {
      return;
   }

   if (key.phenomenon_ != awips::Phenomenon::SevereThunderstorm)
   {
      progressTrack_->setMinimumHeight(0);
      progressTrack_->setMaximumHeight(0);
      progressTrack_->setFixedHeight(0);
      progressTrack_->setVisible(false);
      sweepTimer_->stop();
      return;
   }

   progressTrack_->setMinimumHeight(6);
   progressTrack_->setMaximumHeight(6);
   progressTrack_->setFixedHeight(6);
   progressTrack_->setVisible(true);
   if (!sweepTimer_->isActive())
   {
      sweepPosition_ = -progressSweep_->width();
      sweepTimer_->start();
   }

   const auto begin = segment->event_begin();
   const auto end   = segment->event_end();
   const auto now   = std::chrono::system_clock::now();

   float fractionRemaining = 1.0f;
   if (end > begin)
   {
      const auto totalSeconds =
         std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
      const auto remainingSeconds =
         std::chrono::duration_cast<std::chrono::seconds>(end - now).count();

      if (totalSeconds > 0)
      {
         fractionRemaining = static_cast<float>(remainingSeconds) /
                             static_cast<float>(totalSeconds);
      }
   }

   fractionRemaining = std::clamp(fractionRemaining, 0.0f, 1.0f);

   const int trackWidth = progressTrack_->width();
   const int fillWidth =
      static_cast<int>(static_cast<float>(trackWidth) * fractionRemaining);
   progressFill_->setGeometry(
      0, 0, std::max(0, fillWidth), progressTrack_->height());
}

void WarningBoxWidgetImpl::ApplyTheme(
   const types::TextEventKey&                   key,
   const std::shared_ptr<const awips::Segment>& segment)
{
   std::string accentColor = "197, 37, 48";
   bool        isSevere    = false;

   if (key.phenomenon_ == awips::Phenomenon::SevereThunderstorm)
   {
      // Keep yellow for severe styling, but use a subtler tone.
      accentColor = "191, 151, 58";
      isSevere    = true;
   }
   else if (key.phenomenon_ == awips::Phenomenon::Tornado &&
            (segment->threatCategory_ ==
                awips::ibw::ThreatCategory::Destructive ||
             segment->threatCategory_ ==
                awips::ibw::ThreatCategory::Catastrophic))
   {
      accentColor = "116, 61, 194";
   }
   else if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      accentColor = "197, 37, 48";
   }

   std::string styleSheet;
   styleSheet += "QWidget#WarningBoxWidget {";
   styleSheet += "  background-color: rgba(12, 16, 26, 230);";
   styleSheet += "  border: 2px solid rgba(" + accentColor + ", 220);";
   styleSheet += "  border-radius: 6px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel {";
   styleSheet += "  color: rgb(236, 240, 255);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QScrollArea {";
   styleSheet += "  background: transparent;";
   styleSheet += "  border: none;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QScrollArea > QWidget > QWidget {";
   styleSheet += "  background: transparent;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#areasFrame {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 170);";
   styleSheet += "  background-color: rgba(9, 15, 30, 180);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#areasLabel {";
   styleSheet += "  font-size: 13px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.5px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#statesLabel {";
   styleSheet += "  font-family: '" + monoFontFamily_ + "';";
   styleSheet += "  font-size: 16px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.8px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#stateBadgeFrame {";
   styleSheet += "  border: 1px solid rgba(58, 122, 255, 220);";
   styleSheet += "  border-radius: 3px;";
   styleSheet += "  background-color: rgba(19, 56, 110, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#stateBadgeLabel {";
   styleSheet += "  font-family: '" + monoFontFamily_ + "';";
   styleSheet += "  color: rgb(224, 236, 255);";
   styleSheet += "  font-size: 11px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.8px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressTrack {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 190);";
   styleSheet += "  background-color: rgba(17, 24, 46, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressFill {";
   styleSheet += "  background-color: rgba(235, 191, 66, 255);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressSweep {";
   styleSheet += "  background-color: rgba(247, 252, 255, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#severeMetricCard {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 160);";
   styleSheet += "  background-color: rgba(13, 15, 29, 236);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#severeMetricTitle {";
   styleSheet += "  font-family: '" + monoFontFamily_ + "';";
   styleSheet += "  font-size: 10px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.7px;";
   styleSheet += "  color: rgba(208, 214, 230, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#severeMetricValue {";
   styleSheet += "  font-size: 19px;";
   styleSheet += "  font-weight: 800;";
   styleSheet += "  letter-spacing: 0.6px;";
   styleSheet += "  color: rgba(244, 248, 255, 245);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#detailRow {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 140);";
   styleSheet += "  background-color: rgba(6, 11, 24, 220);";
   styleSheet += "}";
   styleSheet +=
      "QWidget#WarningBoxWidget QFrame#detailRow[highlight=\"true\"] {";
   styleSheet += "  background-color: rgba(27, 67, 92, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#detailLabel {";
   styleSheet += "  font-size: 11px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.6px;";
   styleSheet += "  color: rgba(220, 226, 242, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#detailValue {";
   styleSheet += "  font-size: 12px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.3px;";
   styleSheet += "  color: rgb(236, 240, 255);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#viewEasTextButton {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 200);";
   styleSheet += "  background-color: rgba(2, 6, 14, 235);";
   styleSheet += "  color: rgb(248, 251, 255);";
   styleSheet += "  font-size: 14px;";
   styleSheet += "  font-weight: 800;";
   styleSheet += "  letter-spacing: 0.8px;";
   styleSheet += "  padding: 6px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#closeButton {";
   styleSheet += "  border-radius: 14px;";
   styleSheet += "  border: 1px solid rgba(160, 170, 194, 190);";
   styleSheet += "  background-color: rgba(5, 9, 20, 210);";
   styleSheet += "  color: rgba(235, 240, 255, 230);";
   styleSheet += "  font-size: 15px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#closeButton:hover {";
   styleSheet += "  background-color: rgba(" + accentColor + ", 72);";
   styleSheet += "}";

   if (isSevere)
   {
      // Severe blueprint look: navy body, yellow accents, clean metric cards.
      styleSheet += "QWidget#WarningBoxWidget {";
      styleSheet += "  background-color: rgba(12, 13, 22, 238);";
      styleSheet += "  border: 1px solid rgba(64, 80, 126, 220);";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QFrame#progressTrack {";
      styleSheet += "  border: 1px solid rgba(191, 151, 58, 210);";
      styleSheet += "  background-color: rgba(18, 24, 46, 225);";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QFrame#progressFill {";
      styleSheet += "  background-color: rgba(236, 193, 74, 255);";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QFrame#severeMetricCard {";
      styleSheet += "  border: 1px solid rgba(179, 142, 57, 210);";
      styleSheet += "  border-top: 2px solid rgba(236, 193, 74, 255);";
      styleSheet += "  background-color: rgb(42, 31, 27);";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QLabel#severeMetricTitle {";
      styleSheet += "  font-family: '" + monoFontFamily_ + "';";
      styleSheet += "  color: rgba(202, 186, 147, 235);";
      styleSheet += "  letter-spacing: 0.9px;";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QLabel#severeMetricValue {";
      styleSheet += "  font-size: 18px;";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QFrame#detailRow {";
      styleSheet += "  border: 1px solid rgba(34, 49, 88, 210);";
      styleSheet += "  background-color: rgba(8, 13, 31, 236);";
      styleSheet += "}";
      styleSheet +=
         "QWidget#WarningBoxWidget QFrame#detailRow[highlight=\"true\"] {";
      styleSheet += "  background-color: rgba(17, 31, 66, 235);";
      styleSheet += "}";
   }

   self_->setStyleSheet(QString::fromStdString(styleSheet));
}

void WarningBoxWidgetImpl::UpdateExpirationOnly()
{
   if (currentKey_ == types::TextEventKey {})
   {
      return;
   }

   auto messages = textEventManager_->message_list(currentKey_);
   if (messages.empty())
   {
      return;
   }
   auto segments = messages.back()->segments();
   if (segments.empty())
   {
      return;
   }

   auto eventEnd = segments.back()->event_end();
   auto now      = std::chrono::system_clock::now();
   auto minutes =
      std::chrono::duration_cast<std::chrono::minutes>(eventEnd - now).count();
   std::string expirationStr;
   if (minutes > 0)
   {
      expirationStr =
         fmt::format("EXPIRES IN {} MIN{}", minutes, minutes == 1 ? "" : "S");
   }
   else
   {
      expirationStr = "EXPIRED";
   }

   self_->ui->expirationLabel->setText(QString::fromStdString(expirationStr));
   UpdateProgressVisual(currentKey_, segments.back());
}

void WarningBoxWidgetImpl::UpdateTitleFont()
{
   const int targetWidth = std::max(120, self_->ui->warningTypeLabel->width());
   QFont     font        = self_->ui->warningTypeLabel->font();
   font.setFamily(QString::fromStdString(warningTitleFontFamily_));
   int pointSize = 20;
   font.setPointSize(pointSize);

   while (pointSize > 12)
   {
      QFontMetrics metrics(font);
      if (metrics.horizontalAdvance(self_->ui->warningTypeLabel->text()) <=
          targetWidth)
      {
         break;
      }
      pointSize--;
      font.setPointSize(pointSize);
   }

   self_->ui->warningTypeLabel->setFont(font);
}

void WarningBoxWidgetImpl::AdjustHeightToContents()
{
   if (detailsContainer_ == nullptr)
   {
      return;
   }

   if (fixedWidth_ > 0)
   {
      self_->setMinimumWidth(fixedWidth_);
      self_->setMaximumWidth(fixedWidth_);
      self_->resize(fixedWidth_, self_->height());
   }

   self_->ui->scrollArea->setMinimumHeight(40);
   self_->ui->scrollArea->setMaximumHeight(QWIDGETSIZE_MAX);
   self_->ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

   self_->layout()->activate();
   detailsLayout_->activate();
   detailsContainer_->adjustSize();
   detailsContainer_->updateGeometry();

   static constexpr int kExtraBottomSpace = 8;
   static constexpr int kMinBoxHeight     = 220;
   static constexpr int kMinDetailsHeight = 40;
   static constexpr int kParentPaddingY   = 24;

   int maxHeight = std::max(kMinBoxHeight, self_->sizeHint().height());
   if (self_->parentWidget() != nullptr)
   {
      maxHeight = std::max(kMinBoxHeight,
                           self_->parentWidget()->height() - kParentPaddingY);
   }

   const int detailsWanted =
      std::max(kMinDetailsHeight,
               detailsContainer_->sizeHint().height() +
                  detailsLayout_->contentsMargins().top() +
                  detailsLayout_->contentsMargins().bottom());

   // Estimate non-scroll-area ("chrome") height by pinning scroll area to min
   self_->ui->scrollArea->setMinimumHeight(kMinDetailsHeight);
   self_->ui->scrollArea->setMaximumHeight(kMinDetailsHeight);
   self_->layout()->activate();
   const int chromeHeight =
      std::max(0, self_->sizeHint().height() - kMinDetailsHeight);

   const int availableForDetails =
      std::max(kMinDetailsHeight, maxHeight - chromeHeight - kExtraBottomSpace);
   const int detailsFinalHeight = std::min(detailsWanted, availableForDetails);
   const int panelFinalHeight =
      std::clamp(chromeHeight + detailsFinalHeight + kExtraBottomSpace,
                 kMinBoxHeight,
                 maxHeight);

   self_->ui->scrollArea->setMinimumHeight(detailsFinalHeight);
   self_->ui->scrollArea->setMaximumHeight(detailsFinalHeight);
   self_->ui->scrollArea->setVerticalScrollBarPolicy(
      detailsWanted > detailsFinalHeight ? Qt::ScrollBarAsNeeded :
                                           Qt::ScrollBarAlwaysOff);

   self_->setMinimumHeight(panelFinalHeight);
   self_->setMaximumHeight(panelFinalHeight);
   self_->resize(self_->width(), panelFinalHeight);
   self_->layout()->activate();

   // Re-apply title scaling after final geometry is set.
   UpdateTitleFont();
}

std::string WarningBoxWidgetImpl::ToUpper(std::string_view value)
{
   std::string result(value);
   std::transform(result.begin(),
                  result.end(),
                  result.begin(),
                  [](unsigned char c)
                  { return static_cast<char>(std::toupper(c)); });
   return result;
}

std::string WarningBoxWidgetImpl::Trim(std::string_view value)
{
   const size_t start = value.find_first_not_of(" \t\r\n*");
   if (start == std::string_view::npos)
   {
      return {};
   }

   const size_t end = value.find_last_not_of(" \t\r\n");
   return std::string(value.substr(start,
                                   end == std::string_view::npos ?
                                      std::string_view::npos :
                                      (end - start + 1)));
}

std::string WarningBoxWidgetImpl::NormalizeLabel(std::string_view label)
{
   std::string normalized;
   normalized.reserve(label.size());

   bool previousSpace = false;
   for (char c : label)
   {
      const unsigned char uc = static_cast<unsigned char>(c);
      if (std::isalnum(uc))
      {
         normalized.push_back(static_cast<char>(std::toupper(uc)));
         previousSpace = false;
      }
      else if (!previousSpace)
      {
         normalized.push_back(' ');
         previousSpace = true;
      }
   }

   return Trim(normalized);
}

bool WarningBoxWidgetImpl::IsLabelLine(std::string_view line)
{
   bool hasAlpha = false;

   for (char c : line)
   {
      const unsigned char uc = static_cast<unsigned char>(c);
      if (std::isalpha(uc))
      {
         hasAlpha = true;
      }
      else if (!(std::isdigit(uc) || std::isspace(uc) || c == '/' || c == '-'))
      {
         return false;
      }
   }

   return hasAlpha;
}

std::unordered_map<std::string, std::string>
WarningBoxWidgetImpl::ParseProductFields(
   const std::shared_ptr<const awips::Segment>& segment)
{
   std::unordered_map<std::string, std::string> fields {};
   std::string                                  currentLabel {};

   for (const std::string& rawLine : segment->productContent_)
   {
      const std::string line = Trim(rawLine);
      if (line.empty())
      {
         currentLabel.clear();
         continue;
      }

      if (line.starts_with("LAT...LON") ||
          line.starts_with("TIME...MOT...LOC") || line.starts_with("$$"))
      {
         continue;
      }

      if (!line.empty() &&
          std::isdigit(static_cast<unsigned char>(line.front())) &&
          line.find_first_not_of(" 0123456789") == std::string::npos)
      {
         continue;
      }

      const size_t dottedSep = line.find("...");
      if (dottedSep != std::string::npos)
      {
         const std::string label = Trim(line.substr(0, dottedSep));
         if (!label.empty() && IsLabelLine(label))
         {
            currentLabel         = NormalizeLabel(label);
            fields[currentLabel] = Trim(line.substr(dottedSep + 3));
            continue;
         }
      }

      const size_t colonSep = line.find(": ");
      if (colonSep != std::string::npos)
      {
         const std::string label = Trim(line.substr(0, colonSep));
         if (!label.empty() && IsLabelLine(label))
         {
            currentLabel         = NormalizeLabel(label);
            fields[currentLabel] = Trim(line.substr(colonSep + 2));
            continue;
         }
      }

      if (!currentLabel.empty())
      {
         std::string& value = fields[currentLabel];
         if (!value.empty())
         {
            value += ' ';
         }
         value += line;
      }
   }

   return fields;
}

std::string WarningBoxWidgetImpl::GetFieldValue(
   const std::unordered_map<std::string, std::string>& fields,
   const std::initializer_list<std::string_view>&      keys)
{
   for (std::string_view key : keys)
   {
      const auto it = fields.find(NormalizeLabel(key));
      if (it != fields.cend() && !it->second.empty())
      {
         return it->second;
      }
   }

   return {};
}

void WarningBoxWidgetImpl::AddSummaryField(
   const std::string&                                  summaryLabel,
   const std::unordered_map<std::string, std::string>& fields,
   const std::initializer_list<std::string_view>&      keys)
{
   const std::string value = GetFieldValue(fields, keys);
   if (!value.empty())
   {
      AddDetailRow(summaryLabel, value);
   }
}

void WarningBoxWidgetImpl::AddPhenomenonSpecificFields(
   awips::Phenomenon                                   phenomenon,
   const std::unordered_map<std::string, std::string>& fields)
{
   if (phenomenon == awips::Phenomenon::SevereThunderstorm)
   {
      AddSummaryField("Max Hail Size", fields, {"MAX HAIL SIZE"});
      AddSummaryField("Max Wind Gust", fields, {"MAX WIND GUST"});
      AddSummaryField(
         "Damage", fields, {"THUNDERSTORM DAMAGE THREAT", "DAMAGE THREAT"});
   }
   else if (phenomenon == awips::Phenomenon::Tornado)
   {
      AddSummaryField("Max Hail Size", fields, {"MAX HAIL SIZE"});
      AddSummaryField("Max Wind Gust", fields, {"MAX WIND GUST"});
      AddSummaryField(
         "Damage Threat", fields, {"TORNADO DAMAGE THREAT", "DAMAGE THREAT"});
      AddSummaryField("Tornado Threat", fields, {"TORNADO THREAT"});
   }
}

void WarningBoxWidgetImpl::UpdateCountdown()
{
   UpdateExpirationOnly();
}

#include "warning_box_widget.moc"

} // namespace ui
} // namespace qt
} // namespace scwx
