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
#include <QPolygon>
#include <QRegion>
#include <QScrollBar>

#include <algorithm>
#include <array>
#include <cstdio>
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
   enum class TornadoStyle
   {
      None,
      Normal,
      Pds,
      Emergency
   };

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
                             QTimer::singleShot(0,
                                                self_,
                                                [this]()
                                                {
                                                   if (self_->isVisible())
                                                   {
                                                      AdjustHeightToContents();
                                                      UpdateTitleFont();
                                                      UpdateExpirationOnly();
                                                   }
                                                });
                          }
                       });

      sweepTimer_->setSingleShot(false);
      sweepTimer_->setInterval(33);
      QObject::connect(
         sweepTimer_,
         &QTimer::timeout,
         this,
         [this]()
         {
            if (progressTrack_ == nullptr || progressSweep_ == nullptr ||
                progressSweepGlow_ == nullptr || !progressTrack_->isVisible())
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
            const int glowWidth = progressSweepGlow_->width();
            const int glowY =
               progressSweep_->y() +
               ((progressSweep_->height() - progressSweepGlow_->height()) / 2);
            progressSweepGlow_->move(
               sweepPosition_ - ((glowWidth - sweepWidth) / 2), glowY);
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
   bool AddSevereTornadoPossibleBox(
      const std::unordered_map<std::string, std::string>& fields);
   bool AddSevereDamageThreatBox(
      const std::unordered_map<std::string, std::string>& fields);
   bool AddTornadoDamageThreatBox(
      const std::unordered_map<std::string, std::string>& fields);
   bool               AddTornadoConfirmedBox();
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
   QFrame*                                    progressSweepGlow_ {nullptr};
   QFrame*                                    progressMid_ {nullptr};
   int                                        sweepPosition_ {0};
   int                                        fixedWidth_ {0};
   QWidget*                                   detailsContainer_ {nullptr};
   QVBoxLayout*                               detailsLayout_ {nullptr};
   QFrame*                                    cornerTL_ {nullptr};
   QFrame*                                    cornerTR_ {nullptr};
   QFrame*                                    cornerBL_ {nullptr};
   QFrame*                                    cornerBR_ {nullptr};
   std::string  warningTitleFontFamily_ {"Rajdhani"};
   std::string  expirationFontFamily_ {"IBMPlexMono-SemiBold"};
   std::string  monoFontFamily_ {"RobotoMono-Regular"};
   std::string  areaSourceLabelFontFamily_ {"AlegreyaSans-ExtraBold"};
   std::string  areaSourceValueFontFamily_ {"Rajdhani"};
   TornadoStyle tornadoStyle_ {TornadoStyle::None};
   bool         tornadoObserved_ {false};
};

WarningBoxWidget::WarningBoxWidget(QWidget* parent) :
    QWidget(parent),
    p {std::make_unique<WarningBoxWidgetImpl>(this)},
    ui(new Ui::WarningBoxWidget)
{
   ui->setupUi(this);

   p->warningTitleFontFamily_ =
      LoadFontFamily(":/res/fonts/Rajdhani-Regular.ttf", "Rajdhani");
   LoadFontFamily(":/res/fonts/Rajdhani-Medium.ttf",
                  p->warningTitleFontFamily_.c_str());
   LoadFontFamily(":/res/fonts/Rajdhani-SemiBold.ttf",
                  p->warningTitleFontFamily_.c_str());
   LoadFontFamily(":/res/fonts/Rajdhani-Bold.ttf",
                  p->warningTitleFontFamily_.c_str());
   p->expirationFontFamily_ = LoadFontFamily(
      ":/res/fonts/IBMPlexMono-SemiBold.ttf", "IBMPlexMono-SemiBold");
   p->monoFontFamily_ = LoadFontFamily(":/res/fonts/RobotoMono-Regular.ttf",
                                       "RobotoMono-Regular");
   p->areaSourceLabelFontFamily_ = LoadFontFamily(
      ":/res/fonts/AlegreyaSans-ExtraBold.ttf", "AlegreyaSans-ExtraBold");
   p->areaSourceValueFontFamily_ = p->warningTitleFontFamily_;
   LoadFontFamily(":/res/fonts/AlegreyaSans-Black.ttf", "AlegreyaSans-Black");

   setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground, false);
   setAttribute(Qt::WA_StyledBackground, true);
   setAutoFillBackground(true);

   ui->warningTypeLabel->setTextInteractionFlags(Qt::NoTextInteraction);
   ui->warningTypeLabel->setWordWrap(true);
   ui->warningTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   // Clear .ui stylesheet font sizing so runtime QFont settings are honored.
   ui->warningTypeLabel->setStyleSheet(QString {});
   ui->warningTypeLabel->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Minimum);
   QFont warningTypeFont {};
   warningTypeFont.setFamily(
      QString::fromStdString(p->warningTitleFontFamily_));
   warningTypeFont.setPointSize(19);
   warningTypeFont.setWeight(QFont::Bold);
   warningTypeFont.setStyleStrategy(QFont::PreferAntialias);
   warningTypeFont.setHintingPreference(QFont::PreferNoHinting);
   ui->warningTypeLabel->setFont(warningTypeFont);
   ui->warningTypeLabel->setContentsMargins(0, 0, 0, 0);
   ui->expirationLabel->setStyleSheet(
      QString("font-family: '%1';"
              "font-size: 13px; font-weight: 700; letter-spacing: 0.5px;")
         .arg(QString::fromStdString(p->expirationFontFamily_)));
   ui->expirationLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   ui->expirationLabel->setSizePolicy(QSizePolicy::Preferred,
                                      QSizePolicy::Fixed);
   ui->expirationLabel->setContentsMargins(0, 0, 0, 0);
   ui->viewEasTextButton->setText("VIEW FULL EAS TEXT");
   ui->closeButton->setText("x");
   ui->closeButton->setFixedSize(18, 18);
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

   ui->verticalLayout->removeWidget(ui->warningTypeLabel);
   ui->verticalLayout->removeWidget(ui->expirationLabel);
   QVBoxLayout* titleLayout = new QVBoxLayout();
   titleLayout->setContentsMargins(0, 0, 0, 0);
   titleLayout->setSpacing(0);
   titleLayout->addWidget(ui->warningTypeLabel, 0, Qt::AlignLeft);
   titleLayout->addWidget(ui->expirationLabel, 0, Qt::AlignLeft);
   QHBoxLayout* topLayout = new QHBoxLayout();
   topLayout->setContentsMargins(0, 0, 0, 0);
   topLayout->setSpacing(2);
   topLayout->addLayout(titleLayout, 1);
   topLayout->addWidget(ui->closeButton, 0, Qt::AlignTop);
   ui->verticalLayout->insertLayout(0, topLayout);

   p->progressTrack_ = new QFrame(this);
   p->progressTrack_->setObjectName("progressTrack");
   p->progressTrack_->setFixedHeight(14);
   p->progressTrack_->setVisible(false);
   p->progressFill_ = new QFrame(p->progressTrack_);
   p->progressFill_->setObjectName("progressFill");
   p->progressFill_->setGeometry(0, 0, 0, 6);
   p->progressSweep_ = new QFrame(p->progressTrack_);
   p->progressSweep_->setObjectName("progressSweep");
   p->progressSweep_->setFixedSize(68, 6);
   p->progressSweep_->move(-p->progressSweep_->width(), 0);
   p->progressSweepGlow_ = new QFrame(p->progressTrack_);
   p->progressSweepGlow_->setObjectName("progressSweepGlow");
   p->progressSweepGlow_->setFixedSize(108, 10);
   const int initialGlowY =
      p->progressSweep_->y() +
      ((p->progressSweep_->height() - p->progressSweepGlow_->height()) / 2);
   p->progressSweepGlow_->move(-p->progressSweep_->width() - 20, initialGlowY);
   p->progressSweepGlow_->lower();
   p->progressMid_ = new QFrame(p->progressTrack_);
   p->progressMid_->setObjectName("progressMid");
   p->progressMid_->setFixedHeight(1);
   p->progressMid_->move(0, 3);
   p->progressMid_->lower();
   ui->verticalLayout->insertWidget(2, p->progressTrack_);

   p->stateBadgeFrame_ = new QFrame(this);
   p->stateBadgeFrame_->setObjectName("stateBadgeFrame");
   p->stateBadgeFrame_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
   QHBoxLayout* badgeLayout = new QHBoxLayout(p->stateBadgeFrame_);
   badgeLayout->setContentsMargins(8, 4, 8, 4);
   badgeLayout->setSpacing(0);
   p->stateBadgeLabel_ = new QLabel(p->stateBadgeFrame_);
   p->stateBadgeLabel_->setObjectName("stateBadgeLabel");
   p->stateBadgeLabel_->setAlignment(Qt::AlignCenter);
   p->stateBadgeLabel_->setWordWrap(false);
   p->stateBadgeLabel_->setMinimumWidth(28);
   p->stateBadgeLabel_->setMaximumWidth(120);
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
   p->detailsLayout_->setAlignment(Qt::AlignTop);
   ui->scrollArea->setWidget(p->detailsContainer_);
   ui->scrollArea->setWidgetResizable(true);

   p->cornerTL_ = new QFrame(this);
   p->cornerTL_->setObjectName("cornerTL");
   p->cornerTR_ = new QFrame(this);
   p->cornerTR_->setObjectName("cornerTR");
   p->cornerBL_ = new QFrame(this);
   p->cornerBL_->setObjectName("cornerBL");
   p->cornerBR_ = new QFrame(this);
   p->cornerBR_->setObjectName("cornerBR");
   p->cornerTL_->raise();
   p->cornerTR_->raise();
   p->cornerBL_->raise();
   p->cornerBR_->raise();
   p->cornerTR_->setVisible(false);
   p->cornerBL_->setVisible(false);

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
   const auto fields  = ParseProductFields(segment);

   tornadoStyle_ = TornadoStyle::None;
   if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      const std::string tornadoDamageThreat = ToUpper(
         GetFieldValue(fields, {"TORNADO DAMAGE THREAT", "DAMAGE THREAT"}));
      const bool isCatastrophic =
         segment->threatCategory_ == awips::ibw::ThreatCategory::Catastrophic ||
         tornadoDamageThreat.find("CATASTROPHIC") != std::string::npos;
      const bool isPds =
         segment->threatCategory_ == awips::ibw::ThreatCategory::Destructive ||
         tornadoDamageThreat.find("CONSIDERABLE") != std::string::npos ||
         tornadoDamageThreat.find("DESTRUCTIVE") != std::string::npos;
      tornadoStyle_ = isCatastrophic ? TornadoStyle::Emergency :
                      isPds          ? TornadoStyle::Pds :
                                       TornadoStyle::Normal;
   }

   ApplyTheme(key, segment);

   // Title: from phenomenon and significance (e.g. "Tornado Warning")
   std::string phenText = awips::GetPhenomenonText(key.phenomenon_);
   std::string sigText  = awips::GetSignificanceText(key.significance_);
   std::string title    = ToUpper(fmt::format("{} {}", phenText, sigText));
   if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      if (tornadoStyle_ == TornadoStyle::Emergency)
      {
         title = "TORNADO EMERGENCY";
      }
      else if (tornadoStyle_ == TornadoStyle::Pds)
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
      const QString fullStates    = QString::fromStdString(statesStr);
      const QString displayStates = stateBadgeLabel_->fontMetrics().elidedText(
         fullStates, Qt::ElideRight, 120);
      stateBadgeLabel_->setText(displayStates);
      stateBadgeLabel_->setToolTip(displayStates == fullStates ? QString {} :
                                                                 fullStates);
      stateBadgeFrame_->setVisible(true);
   }
   else
   {
      stateBadgeLabel_->setToolTip(QString {});
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

   tornadoObserved_ = false;
   if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      const std::string tornadoField =
         GetFieldValue(fields, {"TORNADO", "TORNADO THREAT"});
      tornadoObserved_ =
         ToUpper(tornadoField).find("OBSERVED") != std::string::npos;
   }

   if (key.phenomenon_ == awips::Phenomenon::SevereThunderstorm)
   {
      const bool hasTornadoPossible = AddSevereTornadoPossibleBox(fields);
      const bool hasDamageThreat    = AddSevereDamageThreatBox(fields);
      if (hasTornadoPossible && hasDamageThreat)
      {
         detailsLayout_->insertSpacing(1, 1);
      }
      if (hasTornadoPossible || hasDamageThreat)
      {
         detailsLayout_->addSpacing(4);
      }
   }
   else if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      const bool hasDamageThreat = AddTornadoDamageThreatBox(fields);
      const bool hasConfirmed    = AddTornadoConfirmedBox();
      if (hasDamageThreat && hasConfirmed)
      {
         detailsLayout_->insertSpacing(1, 1);
      }
      if (hasDamageThreat || hasConfirmed)
      {
         detailsLayout_->addSpacing(4);
      }
   }

   const std::string maxHail = GetFieldValue(fields, {"MAX HAIL SIZE"});
   const std::string maxWind = GetFieldValue(fields, {"MAX WIND GUST"});
   if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      AddSevereMetricCards({}, {});
   }
   else
   {
      AddSevereMetricCards(maxHail, maxWind);
   }

   if (!countiesStr.empty())
   {
      AddDetailRow("Areas", countiesStr);
   }
   if (key.phenomenon_ == awips::Phenomenon::Tornado && !maxWind.empty())
   {
      AddDetailRow("Max Wind Gust", maxWind);
   }
   if (key.phenomenon_ == awips::Phenomenon::Tornado && !maxHail.empty())
   {
      AddDetailRow("Max Hail Size", maxHail);
   }
   AddSummaryField("Source", fields, {"SOURCE"});
   AddPhenomenonSpecificFields(key.phenomenon_, fields);

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
   valueW->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

   labelW->setStyleSheet(
      QString("font-family: '%1'; font-weight: 700; letter-spacing: 0px;")
         .arg(QString::fromStdString(areaSourceLabelFontFamily_)));

   valueW->setStyleSheet(
      QString("font-family: '%1'; font-size: 15px; font-weight: 700; "
              "letter-spacing: 0px;")
         .arg(QString::fromStdString(areaSourceValueFontFamily_)));

   if (label == "Source" && tornadoObserved_)
   {
      auto* sourceGlow = new QGraphicsDropShadowEffect(valueW);
      sourceGlow->setBlurRadius(7.0);
      sourceGlow->setColor(QColor(245, 248, 255, 160));
      sourceGlow->setOffset(0.0, 0.0);
      valueW->setGraphicsEffect(sourceGlow);
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
      [this, rowContainer](const std::string& title, const std::string& value)
   {
      QFrame* card = new QFrame(rowContainer);
      card->setObjectName("severeMetricCard");
      card->setMinimumHeight(54);
      QVBoxLayout* cardLayout = new QVBoxLayout(card);
      cardLayout->setContentsMargins(8, 3, 8, 3);
      cardLayout->setSpacing(0);

      QLabel* titleLabel =
         new QLabel(QString::fromStdString(ToUpper(title)), card);
      titleLabel->setObjectName("severeMetricTitle");
      titleLabel->setContentsMargins(0, 0, 0, 0);
      QLabel* valueLabel = new QLabel(QString::fromStdString(value), card);
      valueLabel->setObjectName("severeMetricValue");
      valueLabel->setContentsMargins(0, 0, 0, 0);
      if (currentKey_.phenomenon_ == awips::Phenomenon::SevereThunderstorm)
      {
         QFont metricValueFont = valueLabel->font();
         metricValueFont.setFamily(
            QString::fromStdString(warningTitleFontFamily_));
         metricValueFont.setPointSize(20);
         metricValueFont.setWeight(QFont::Bold);
         metricValueFont.setStyleStrategy(QFont::PreferAntialias);
         metricValueFont.setHintingPreference(QFont::PreferNoHinting);
         valueLabel->setFont(metricValueFont);
      }
      auto* valueGlow = new QGraphicsDropShadowEffect(valueLabel);
      valueGlow->setBlurRadius(12.0);
      if (currentKey_.phenomenon_ == awips::Phenomenon::Tornado)
      {
         valueGlow->setColor(QColor(220, 56, 56, 220));
      }
      else
      {
         valueGlow->setColor(QColor(236, 193, 74, 210));
      }
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

bool WarningBoxWidgetImpl::AddSevereTornadoPossibleBox(
   const std::unordered_map<std::string, std::string>& fields)
{
   const std::string tornadoField =
      GetFieldValue(fields, {"TORNADO", "TORNADO THREAT"});
   if (tornadoField.empty() ||
       ToUpper(tornadoField).find("POSSIBLE") == std::string::npos)
   {
      return false;
   }

   QFrame* box = new QFrame(detailsContainer_);
   box->setObjectName("severeTornadoPossibleBox");
   box->setMinimumHeight(28);
   box->setMaximumHeight(28);

   QHBoxLayout* layout = new QHBoxLayout(box);
   layout->setContentsMargins(8, 0, 8, 0);
   layout->setSpacing(0);

   QLabel* label = new QLabel("TORNADO POSSIBLE", box);
   label->setObjectName("severeTornadoPossibleValue");
   label->setAlignment(Qt::AlignCenter);
   auto* glow = new QGraphicsDropShadowEffect(label);
   glow->setBlurRadius(14.0);
   glow->setColor(QColor(236, 64, 64, 230));
   glow->setOffset(0.0, 0.0);
   label->setGraphicsEffect(glow);

   layout->addWidget(label);
   detailsLayout_->addWidget(box);
   return true;
}

bool WarningBoxWidgetImpl::AddSevereDamageThreatBox(
   const std::unordered_map<std::string, std::string>& fields)
{
   const std::string damageField =
      GetFieldValue(fields, {"THUNDERSTORM DAMAGE THREAT", "DAMAGE THREAT"});
   const std::string upperDamage = ToUpper(damageField);
   const bool        isConsiderable =
      upperDamage.find("CONSIDERABLE") != std::string::npos;
   const bool isDestructive =
      upperDamage.find("DESTRUCTIVE") != std::string::npos;
   if (!(isConsiderable || isDestructive))
   {
      return false;
   }

   QFrame* box = new QFrame(detailsContainer_);
   box->setObjectName("severeDamageThreatBox");
   box->setMinimumHeight(28);
   box->setMaximumHeight(28);

   QHBoxLayout* layout = new QHBoxLayout(box);
   layout->setContentsMargins(8, 0, 8, 0);
   layout->setSpacing(0);

   const QString text =
      QString::fromStdString(isDestructive ? "DESTRUCTIVE DAMAGE THREAT" :
                                             "CONSIDERABLE DAMAGE THREAT");
   QLabel* label = new QLabel(text, box);
   label->setObjectName("severeDamageThreatValue");
   label->setAlignment(Qt::AlignCenter);
   auto* glow = new QGraphicsDropShadowEffect(label);
   glow->setBlurRadius(14.0);
   glow->setColor(QColor(236, 193, 74, 230));
   glow->setOffset(0.0, 0.0);
   label->setGraphicsEffect(glow);

   layout->addWidget(label);
   detailsLayout_->addWidget(box);
   return true;
}

bool WarningBoxWidgetImpl::AddTornadoDamageThreatBox(
   const std::unordered_map<std::string, std::string>& fields)
{
   if (tornadoStyle_ != TornadoStyle::Pds &&
       tornadoStyle_ != TornadoStyle::Emergency)
   {
      return false;
   }

   const std::string damageField = ToUpper(
      GetFieldValue(fields, {"TORNADO DAMAGE THREAT", "DAMAGE THREAT"}));

   std::string text = "CONSIDERABLE DAMAGE THREAT";
   if (damageField.find("CATASTROPHIC") != std::string::npos)
   {
      text = "CATASTROPHIC DAMAGE THREAT";
   }
   else if (damageField.find("DESTRUCTIVE") != std::string::npos)
   {
      text = "DESTRUCTIVE DAMAGE THREAT";
   }
   else if (damageField.find("CONSIDERABLE") != std::string::npos)
   {
      text = "CONSIDERABLE DAMAGE THREAT";
   }

   QColor glowColor {220, 56, 56, 230};
   if (tornadoStyle_ == TornadoStyle::Pds)
   {
      glowColor = QColor(150, 120, 242, 230);
   }
   else if (tornadoStyle_ == TornadoStyle::Emergency)
   {
      glowColor = QColor(255, 0, 255, 235);
   }

   QFrame* box = new QFrame(detailsContainer_);
   box->setObjectName("tornadoDamageThreatBox");
   box->setMinimumHeight(28);
   box->setMaximumHeight(28);

   QHBoxLayout* layout = new QHBoxLayout(box);
   layout->setContentsMargins(8, 0, 8, 0);
   layout->setSpacing(0);

   QLabel* label = new QLabel(QString::fromStdString(text), box);
   label->setObjectName("tornadoDamageThreatValue");
   label->setAlignment(Qt::AlignCenter);
   auto* glow = new QGraphicsDropShadowEffect(label);
   glow->setBlurRadius(14.0);
   glow->setColor(glowColor);
   glow->setOffset(0.0, 0.0);
   label->setGraphicsEffect(glow);

   layout->addWidget(label);
   detailsLayout_->addWidget(box);
   return true;
}

bool WarningBoxWidgetImpl::AddTornadoConfirmedBox()
{
   if (!tornadoObserved_ ||
       currentKey_.phenomenon_ != awips::Phenomenon::Tornado)
   {
      return false;
   }

   QColor glowColor {220, 56, 56, 230};
   if (tornadoStyle_ == TornadoStyle::Pds)
   {
      glowColor = QColor(150, 120, 242, 230);
   }
   else if (tornadoStyle_ == TornadoStyle::Emergency)
   {
      glowColor = QColor(255, 0, 255, 235);
   }

   QFrame* box = new QFrame(detailsContainer_);
   box->setObjectName("tornadoConfirmedBox");
   box->setMinimumHeight(28);
   box->setMaximumHeight(28);

   QHBoxLayout* layout = new QHBoxLayout(box);
   layout->setContentsMargins(8, 0, 8, 0);
   layout->setSpacing(0);

   QLabel* label = new QLabel("TORNADO CONFIRMED", box);
   label->setObjectName("tornadoConfirmedValue");
   label->setAlignment(Qt::AlignCenter);
   auto* glow = new QGraphicsDropShadowEffect(label);
   glow->setBlurRadius(14.0);
   glow->setColor(glowColor);
   glow->setOffset(0.0, 0.0);
   label->setGraphicsEffect(glow);

   layout->addWidget(label);
   detailsLayout_->addWidget(box);
   return true;
}

void WarningBoxWidgetImpl::UpdateProgressVisual(
   const types::TextEventKey&                   key,
   const std::shared_ptr<const awips::Segment>& segment)
{
   if (progressTrack_ == nullptr || progressFill_ == nullptr ||
       progressSweep_ == nullptr || progressMid_ == nullptr)
   {
      return;
   }

   static_cast<void>(key);

   progressTrack_->setMinimumHeight(14);
   progressTrack_->setMaximumHeight(14);
   progressTrack_->setFixedHeight(14);
   progressTrack_->setVisible(true);
   if (!sweepTimer_->isActive())
   {
      sweepPosition_ = -progressSweep_->width();
      sweepTimer_->start();
   }

   const auto begin = segment->event_begin();
   const auto end   = segment->event_end();
   const auto now   = std::chrono::system_clock::now();

   float fractionElapsed = 0.0f;
   if (end > begin)
   {
      const auto totalSeconds =
         std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
      const auto elapsedSeconds =
         std::chrono::duration_cast<std::chrono::seconds>(now - begin).count();

      if (totalSeconds > 0)
      {
         fractionElapsed = static_cast<float>(elapsedSeconds) /
                           static_cast<float>(totalSeconds);
      }
   }

   fractionElapsed = std::clamp(fractionElapsed, 0.0f, 1.0f);

   const int trackWidth  = progressTrack_->width();
   const int trackHeight = progressTrack_->height();
   const int fillHeight  = 6;
   progressSweep_->setFixedHeight(fillHeight);
   const int midHeight = progressMid_->height();
   static_cast<void>(trackHeight);
   progressMid_->setGeometry(0, 3, trackWidth, midHeight);
   const int fillWidth =
      static_cast<int>(static_cast<float>(trackWidth) * fractionElapsed);
   progressFill_->setGeometry(0, 0, std::max(0, fillWidth), fillHeight);
}

void WarningBoxWidgetImpl::ApplyTheme(
   const types::TextEventKey&                   key,
   const std::shared_ptr<const awips::Segment>& segment)
{
   static_cast<void>(segment);
   std::string accentColor = "197, 37, 48";
   std::string cornerGlowColor {"197, 37, 48"};
   std::string transitionGlowColor {"40, 24, 32"};
   bool        isSevere  = false;
   bool        isTornado = false;

   if (key.phenomenon_ == awips::Phenomenon::SevereThunderstorm)
   {
      // Keep yellow for severe styling, but use a subtler tone.
      accentColor         = "191, 151, 58";
      cornerGlowColor     = "47, 40, 39"; // #2f2827
      transitionGlowColor = "68, 55, 40";
      isSevere            = true;
   }
   else if (key.phenomenon_ == awips::Phenomenon::Tornado)
   {
      cornerGlowColor     = "60, 16, 32"; // normal tornado: deep red-magenta
      transitionGlowColor = "92, 24, 56";
      if (tornadoStyle_ == TornadoStyle::Emergency)
      {
         accentColor         = "255, 0, 255";
         cornerGlowColor     = "255, 0, 255"; // #ff00ff
         transitionGlowColor = "200, 0, 200";
      }
      else if (tornadoStyle_ == TornadoStyle::Pds)
      {
         accentColor         = "116, 61, 194";
         cornerGlowColor     = "60, 20, 87"; // #3c1457
         transitionGlowColor = "90, 45, 120";
      }
      else
      {
         accentColor = "197, 37, 48";
      }
      isTornado = true;
   }
   const bool isBaseTornado = (key.phenomenon_ == awips::Phenomenon::Tornado &&
                               tornadoStyle_ == TornadoStyle::Normal);

   std::string tornadoBoxBackground = "62, 20, 20";
   std::string tornadoBoxTextColor  = "255, 232, 232";
   if (tornadoStyle_ == TornadoStyle::Pds)
   {
      tornadoBoxBackground = "49, 27, 78";
      tornadoBoxTextColor  = "238, 232, 255";
   }
   else if (tornadoStyle_ == TornadoStyle::Emergency)
   {
      tornadoBoxBackground = "64, 0, 64";
      tornadoBoxTextColor  = "255, 230, 255";
   }

   if (stateBadgeFrame_ != nullptr)
   {
      if (isSevere)
      {
         stateBadgeFrame_->setFixedHeight(22);
      }
      else
      {
         stateBadgeFrame_->setFixedHeight(26);
      }

      if (auto* badgeLayout =
             qobject_cast<QHBoxLayout*>(stateBadgeFrame_->layout());
          badgeLayout != nullptr)
      {
         if (isSevere)
         {
            badgeLayout->setContentsMargins(5, 1, 5, 1);
         }
         else
         {
            badgeLayout->setContentsMargins(7, 2, 7, 2);
         }
      }
   }

   std::string styleSheet;
   styleSheet += "QWidget#WarningBoxWidget {";
   if (isBaseTornado)
   {
      styleSheet +=
         "  background-color: qradialgradient("
         "cx:1.0, cy:0.0, radius:0.88, fx:1.0, fy:0.0, "
         "stop:0 rgba(" +
         cornerGlowColor +
         ", 250), "
         "stop:0.25 rgba(43, 17, 32, 255), "
         "stop:0.38 rgba(42, 17, 32, 255), "
         "stop:0.50 rgba(41, 17, 32, 255), "
         "stop:0.75 rgba(30, 12, 30, 255), "
         "stop:1 rgba(0, 2, 26, 255));";
   }
   else if (isSevere)
   {
      styleSheet +=
         "  background-color: qradialgradient("
         "cx:1.0, cy:0.0, radius:0.88, fx:1.0, fy:0.0, "
         "stop:0 rgba(" +
         cornerGlowColor +
         ", 250), "
         "stop:0.25 rgba(47, 41, 38, 255), "
         "stop:0.33 rgba(" +
         cornerGlowColor +
         ", 250), "
         "stop:0.50 rgba(37, 33, 36, 255), "
         "stop:0.75 rgba(28, 26, 33, 255), "
         "stop:1 rgba(0, 2, 26, 255));";
   }
   else
   {
      styleSheet +=
         "  background-color: qradialgradient("
         "cx:1.0, cy:0.0, radius:0.88, fx:1.0, fy:0.0, "
         "stop:0 rgba(" +
         cornerGlowColor +
         ", 255), "
         "stop:0.20 rgba(" +
         transitionGlowColor +
         ", 252), "
         "stop:0.38 rgba(" +
         transitionGlowColor +
         ", 240), "
         "stop:0.54 rgba(38, 20, 33, 255), "
         "stop:0.66 rgba(0, 2, 26, 255), "
         "stop:0.76 rgba(0, 2, 26, 255), "
         "stop:1 rgba(0, 2, 26, 255));";
   }
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 220);";
   styleSheet += "  border-radius: 0px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#cornerTL {";
   styleSheet += "  border-top: 2px solid rgba(" + accentColor + ", 255);";
   styleSheet += "  border-left: 2px solid rgba(" + accentColor + ", 255);";
   styleSheet += "  background: transparent;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#cornerTR {";
   styleSheet += "  border-top: 2px solid rgba(" + accentColor + ", 255);";
   styleSheet += "  border-right: 2px solid rgba(" + accentColor + ", 255);";
   styleSheet += "  background: transparent;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#cornerBL {";
   styleSheet += "  border-bottom: 2px solid rgba(" + accentColor + ", 255);";
   styleSheet += "  border-left: 2px solid rgba(" + accentColor + ", 255);";
   styleSheet += "  background: transparent;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#cornerBR {";
   styleSheet += "  border: none;";
   styleSheet +=
      "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
      "stop:0 rgba(0, 2, 26, 255), "
      "stop:0.44 rgba(0, 2, 26, 255), "
      "stop:0.48 rgba(" +
      accentColor +
      ", 240), "
      "stop:0.50 rgba(" +
      accentColor +
      ", 255), "
      "stop:0.52 rgba(" +
      accentColor +
      ", 240), "
      "stop:0.56 rgba(0, 2, 26, 255), "
      "stop:1 rgba(0, 2, 26, 255));";
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
   styleSheet += "  border: none;";
   styleSheet += "  background-color: transparent;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressFill {";
   styleSheet += "  background-color: rgba(" + accentColor + ", 255);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressSweep {";
   styleSheet += "  background-color: rgba(247, 252, 255, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressSweepGlow {";
   styleSheet +=
      "  background: qlineargradient(x1:0, y1:0.5, x2:1, y2:0.5, "
      "stop:0 rgba(255, 255, 255, 0), "
      "stop:0.28 rgba(255, 255, 255, 20), "
      "stop:0.5 rgba(255, 255, 255, 52), "
      "stop:0.72 rgba(255, 255, 255, 20), "
      "stop:1 rgba(255, 255, 255, 0));";
   styleSheet += "  border-radius: 5px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#progressMid {";
   styleSheet += "  background-color: rgb(96, 99, 105);";
   styleSheet += "  border-radius: 1px;";
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
   styleSheet += "  font-weight: 900;";
   styleSheet += "  letter-spacing: 0.6px;";
   styleSheet += "  color: rgba(244, 248, 255, 245);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#severeTornadoPossibleBox {";
   styleSheet += "  border: 1px solid rgba(210, 62, 62, 225);";
   styleSheet += "  background-color: rgba(62, 20, 20, 235);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#severeTornadoPossibleValue {";
   styleSheet += "  font-size: 19px;";
   styleSheet += "  font-weight: 900;";
   styleSheet += "  letter-spacing: 1.8px;";
   styleSheet += "  color: rgba(255, 232, 232, 250);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#severeDamageThreatBox {";
   styleSheet += "  border: 1px solid rgba(191, 151, 58, 235);";
   styleSheet += "  background-color: rgba(61, 45, 21, 235);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#severeDamageThreatValue {";
   styleSheet += "  font-size: 19px;";
   styleSheet += "  font-weight: 900;";
   styleSheet += "  letter-spacing: 1.8px;";
   styleSheet += "  color: rgba(255, 246, 214, 250);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#tornadoDamageThreatBox {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 235);";
   styleSheet += "  background-color: rgba(" + tornadoBoxBackground + ", 238);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#tornadoDamageThreatValue {";
   styleSheet += "  font-size: 16px;";
   styleSheet += "  font-weight: 900;";
   styleSheet += "  letter-spacing: 1.6px;";
   styleSheet += "  color: rgba(" + tornadoBoxTextColor + ", 250);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QFrame#tornadoConfirmedBox {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 235);";
   styleSheet += "  background-color: rgba(" + tornadoBoxBackground + ", 238);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#tornadoConfirmedValue {";
   styleSheet += "  font-size: 16px;";
   styleSheet += "  font-weight: 900;";
   styleSheet += "  letter-spacing: 1.6px;";
   styleSheet += "  color: rgba(" + tornadoBoxTextColor + ", 250);";
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
   styleSheet += "  font-family: 'AlegreyaSans-Black';";
   styleSheet += "  font-size: 14px;";
   styleSheet += "  font-weight: 800;";
   styleSheet += "  letter-spacing: 0.8px;";
   styleSheet += "  padding: 6px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#closeButton {";
   styleSheet += "  border: none;";
   styleSheet += "  background: transparent;";
   styleSheet += "  color: rgba(235, 240, 255, 230);";
   styleSheet += "  font-size: 17px;";
   styleSheet += "  font-weight: 800;";
   styleSheet += "  padding: 0px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#closeButton:hover {";
   styleSheet += "  color: rgba(255, 255, 255, 255);";
   styleSheet += "}";

   if (isSevere)
   {
      // Severe blueprint tuning: keep compact typography and card treatment.
      styleSheet += "QWidget#WarningBoxWidget QFrame#progressTrack {";
      styleSheet += "  border: none;";
      styleSheet += "  background-color: transparent;";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QLabel#stateBadgeLabel {";
      styleSheet += "  font-size: 10px;";
      styleSheet += "  letter-spacing: 0.6px;";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QFrame#severeMetricCard {";
      styleSheet += "  border: 1px solid rgba(179, 142, 57, 210);";
      styleSheet += "  border-top: 2px solid rgba(236, 193, 74, 255);";
      styleSheet += "  background-color: rgb(42, 31, 27);";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QLabel#severeMetricTitle {";
      styleSheet += "  font-family: '" + monoFontFamily_ + "';";
      styleSheet += "  font-size: 9px;";
      styleSheet += "  color: rgba(202, 186, 147, 235);";
      styleSheet += "  letter-spacing: 0.9px;";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QLabel#severeMetricValue {";
      styleSheet += "  font-family: '" + warningTitleFontFamily_ + "';";
      styleSheet += "  font-size: 20px;";
      styleSheet += "  font-weight: 700;";
      styleSheet += "}";
      styleSheet +=
         "QWidget#WarningBoxWidget QFrame#severeTornadoPossibleBox {";
      styleSheet += "  border: 1px solid rgba(220, 72, 72, 235);";
      styleSheet += "  background-color: rgba(73, 21, 21, 238);";
      styleSheet += "}";
      styleSheet +=
         "QWidget#WarningBoxWidget QLabel#severeTornadoPossibleValue {";
      styleSheet += "  font-size: 16px;";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QFrame#severeDamageThreatBox {";
      styleSheet += "  border: 1px solid rgba(205, 166, 75, 235);";
      styleSheet += "  background-color: rgba(67, 49, 20, 238);";
      styleSheet += "}";
      styleSheet += "QWidget#WarningBoxWidget QLabel#severeDamageThreatValue {";
      styleSheet += "  font-size: 16px;";
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

   if (isTornado)
   {
      // Tornado metric cards: dark maroon body with brighter maroon top strip.
      styleSheet += "QWidget#WarningBoxWidget QFrame#severeMetricCard {";
      styleSheet += "  border: 1px solid rgba(132, 18, 8, 210);";
      styleSheet += "  border-top: 2px solid rgb(132, 18, 8);";
      styleSheet += "  background-color: rgb(44, 6, 24);";
      styleSheet += "}";

      // Tornado parameter rows: match severe row colors (no red border).
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

   int ar = 0;
   int ag = 0;
   int ab = 0;
#if defined(_MSC_VER)
   (void) ::sscanf_s(accentColor.c_str(), "%d, %d, %d", &ar, &ag, &ab);
#else
   (void) std::sscanf(accentColor.c_str(), "%d, %d, %d", &ar, &ag, &ab);
#endif
   const QColor accentGlowColor {ar, ag, ab, 60};
   if (cornerTL_ != nullptr)
   {
      auto* glow = new QGraphicsDropShadowEffect(cornerTL_);
      glow->setBlurRadius(6.0);
      glow->setColor(accentGlowColor);
      glow->setOffset(0.0, 0.0);
      cornerTL_->setGraphicsEffect(glow);
   }
   if (cornerBR_ != nullptr)
   {
      auto* glow = new QGraphicsDropShadowEffect(cornerBR_);
      glow->setBlurRadius(6.0);
      glow->setColor(accentGlowColor);
      glow->setOffset(0.0, 0.0);
      cornerBR_->setGraphicsEffect(glow);
   }
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
   const int expirationHeight =
      self_->ui->expirationLabel->fontMetrics().height();
   self_->ui->expirationLabel->setMinimumHeight(expirationHeight);
   self_->ui->expirationLabel->setMaximumHeight(expirationHeight);
   UpdateProgressVisual(currentKey_, segments.back());
}

void WarningBoxWidgetImpl::UpdateTitleFont()
{
   const QMargins margins = self_->ui->verticalLayout->contentsMargins();
   int targetWidth        = self_->width() - margins.left() - margins.right();
   targetWidth -= self_->ui->closeButton->width();
   targetWidth -= 2; // topLayout spacing between title block and close button
   targetWidth = std::max(120, targetWidth);

   QFont font = self_->ui->warningTypeLabel->font();
   font.setFamily(QString::fromStdString(warningTitleFontFamily_));
   font.setWeight(QFont::Bold);
   font.setStyleStrategy(QFont::PreferAntialias);
   font.setHintingPreference(QFont::PreferNoHinting);
   const QString text = self_->ui->warningTypeLabel->text();

   font.setPointSize(19);
   QFontMetrics finalMetrics(font);
   const QRect  textBounds = finalMetrics.boundingRect(
      QRect(0, 0, targetWidth, 1000), Qt::TextWordWrap, text);
   const int titleHeight =
      std::max(finalMetrics.lineSpacing(), textBounds.height());

   self_->ui->warningTypeLabel->setFont(font);
   self_->ui->warningTypeLabel->setMinimumWidth(targetWidth);
   self_->ui->warningTypeLabel->setMaximumWidth(targetWidth);
   self_->ui->warningTypeLabel->setFixedHeight(titleHeight);
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

   self_->ui->scrollArea->setMinimumHeight(20);
   self_->ui->scrollArea->setMaximumHeight(QWIDGETSIZE_MAX);
   self_->ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

   self_->layout()->activate();
   detailsLayout_->activate();
   detailsContainer_->setMinimumHeight(0);
   detailsContainer_->setMaximumHeight(QWIDGETSIZE_MAX);
   detailsContainer_->adjustSize();
   detailsContainer_->updateGeometry();

   static constexpr int kExtraBottomSpace = 2;
   static constexpr int kMinBoxHeight     = 0;
   static constexpr int kMinDetailsHeight = 20;
   static constexpr int kParentPaddingY   = 24;

   int maxHeight = std::max(kMinBoxHeight, self_->sizeHint().height());
   if (self_->parentWidget() != nullptr)
   {
      maxHeight = std::max(kMinBoxHeight,
                           self_->parentWidget()->height() - kParentPaddingY);
   }

   const int detailsWanted = std::max(
      kMinDetailsHeight, detailsContainer_->minimumSizeHint().height());

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

   static constexpr int kCornerSize = 12;
   const int            w           = self_->width();
   const int            h           = self_->height();

   if (w > (kCornerSize + 2) && h > (kCornerSize + 2))
   {
      const int cut = kCornerSize + 1;
      QRegion   panelRegion(0, 0, w, h);
      QPolygon  cutPolygon;
      cutPolygon << QPoint(w - cut, h - 1) << QPoint(w - 1, h - cut)
                 << QPoint(w - 1, h - 1);
      panelRegion = panelRegion.subtracted(QRegion(cutPolygon));
      self_->setMask(panelRegion);
   }
   else
   {
      self_->clearMask();
   }

   if (cornerTL_ != nullptr && cornerTR_ != nullptr && cornerBL_ != nullptr &&
       cornerBR_ != nullptr)
   {
      cornerTL_->setGeometry(0, 0, kCornerSize, kCornerSize);
      cornerTR_->setGeometry(w - kCornerSize, 0, kCornerSize, kCornerSize);
      cornerBL_->setGeometry(0, h - kCornerSize, kCornerSize, kCornerSize);
      cornerBR_->setGeometry(
         w - kCornerSize, h - kCornerSize, kCornerSize, kCornerSize);
   }

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
      // Severe warning metric boxes now handle hail/wind/damage presentation.
   }
   else if (phenomenon == awips::Phenomenon::Tornado)
   {
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
