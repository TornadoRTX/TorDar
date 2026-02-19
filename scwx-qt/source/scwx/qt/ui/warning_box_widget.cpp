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

class WarningBoxWidgetImpl : public QObject
{
   Q_OBJECT

public:
   explicit WarningBoxWidgetImpl(WarningBoxWidget* self) :
       self_ {self},
       textEventManager_ {manager::TextEventManager::Instance()},
       alertDialog_ {nullptr},
       updateTimer_ {new QTimer(self)},
       currentKey_ {}
   {
      updateTimer_->setSingleShot(false);
      updateTimer_->setInterval(1000);
      QObject::connect(updateTimer_,
                       &QTimer::timeout,
                       this,
                       &WarningBoxWidgetImpl::UpdateCountdown);
   }
   ~WarningBoxWidgetImpl() = default;

   void PopulateFromWarning(const types::TextEventKey& key);
   void UpdateCountdown();
   void ApplyTheme(const types::TextEventKey&                   key,
                   const std::shared_ptr<const awips::Segment>& segment);
   void AddDetailRow(const std::string& label, const std::string& value);
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
   types::TextEventKey                        currentKey_;
   QWidget*                                   detailsContainer_ {nullptr};
   QVBoxLayout*                               detailsLayout_ {nullptr};
};

WarningBoxWidget::WarningBoxWidget(QWidget* parent) :
    QWidget(parent),
    p {std::make_unique<WarningBoxWidgetImpl>(this)},
    ui(new Ui::WarningBoxWidget)
{
   ui->setupUi(this);

   setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground, false);
   setAttribute(Qt::WA_StyledBackground, true);
   setAutoFillBackground(true);

   ui->warningTypeLabel->setTextInteractionFlags(Qt::NoTextInteraction);
   ui->warningTypeLabel->setStyleSheet(
      "font-size: 24px; font-weight: 800; letter-spacing: 1px;");
   ui->expirationLabel->setStyleSheet(
      "font-size: 15px; font-weight: 700; letter-spacing: 0.5px;");
   ui->viewEasTextButton->setText("VIEW FULL EAS TEXT");
   ui->closeButton->setText("x");
   ui->closeButton->setFixedSize(34, 34);
   ui->buttonsLayout->setContentsMargins(0, 0, 0, 0);
   ui->buttonsLayout->setSpacing(8);
   ui->verticalLayout->setContentsMargins(12, 12, 12, 12);
   ui->verticalLayout->setSpacing(8);

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
}

void WarningBoxWidget::HideWarning()
{
   p->updateTimer_->stop();
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

   auto& message  = messages.back();
   auto  segments = message->segments();
   if (segments.empty())
      return;

   auto& segment = segments.back();
   ApplyTheme(key, segment);

   // Title: from phenomenon and significance (e.g. "Tornado Warning")
   std::string phenText = awips::GetPhenomenonText(key.phenomenon_);
   std::string sigText  = awips::GetSignificanceText(key.significance_);
   std::string title    = ToUpper(fmt::format("{} {}", phenText, sigText));
   self_->ui->warningTypeLabel->setText(QString::fromStdString(title));

   // Expiration: from event end
   auto eventEnd = segment->event_end();
   auto now      = std::chrono::system_clock::now();
   auto minutes =
      std::chrono::duration_cast<std::chrono::minutes>(eventEnd - now).count();
   std::string expirationStr;
   if (minutes > 0)
      expirationStr =
         fmt::format("EXPIRES IN {} MIN{}", minutes, minutes == 1 ? "" : "S");
   else
      expirationStr = "EXPIRED";
   self_->ui->expirationLabel->setText(QString::fromStdString(expirationStr));

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
   if (!countiesStr.empty() || !statesStr.empty())
   {
      self_->ui->areasFrame->setVisible(true);
      if (!countiesStr.empty())
      {
         self_->ui->areasLabel->setVisible(true);
         self_->ui->areasLabel->setText(
            QString::fromStdString(fmt::format("AREAS  {}", countiesStr)));
      }
      else
         self_->ui->areasLabel->setVisible(false);
      if (!statesStr.empty())
      {
         self_->ui->statesLabel->setVisible(true);
         self_->ui->statesLabel->setText(QString::fromStdString(statesStr));
      }
      else
         self_->ui->statesLabel->setVisible(false);
   }
   else
   {
      self_->ui->areasFrame->setVisible(false);
   }

   // Clear previous detail rows
   while (QLayoutItem* item = detailsLayout_->takeAt(0))
   {
      if (item->widget())
         item->widget()->deleteLater();
      delete item;
   }

   const auto fields = ParseProductFields(segment);

   AddSummaryField("Hazard", fields, {"HAZARD"});
   AddSummaryField("Source", fields, {"SOURCE"});
   AddSummaryField("Impact", fields, {"IMPACT"});
   AddPhenomenonSpecificFields(key.phenomenon_, fields);

   detailsLayout_->addStretch();
}

void WarningBoxWidgetImpl::AddDetailRow(const std::string& label,
                                        const std::string& value)
{
   QFrame* rowFrame = new QFrame(self_);
   rowFrame->setObjectName("detailRow");
   rowFrame->setProperty("highlight", label == "Source");

   QLabel* labelW = new QLabel(QString::fromStdString(ToUpper(label)));
   labelW->setObjectName("detailLabel");

   QLabel* valueW = new QLabel(QString::fromStdString(value));
   valueW->setObjectName("detailValue");
   valueW->setWordWrap(true);

   QHBoxLayout* row = new QHBoxLayout(rowFrame);
   row->setContentsMargins(10, 8, 10, 8);
   row->setSpacing(8);
   row->addWidget(labelW, 0);
   row->addWidget(valueW, 1);

   detailsLayout_->addWidget(rowFrame);
}

void WarningBoxWidgetImpl::ApplyTheme(
   const types::TextEventKey&                   key,
   const std::shared_ptr<const awips::Segment>& segment)
{
   std::string accentColor = "197, 37, 48";

   if (key.phenomenon_ == awips::Phenomenon::SevereThunderstorm)
   {
      accentColor = "214, 163, 0";
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
   styleSheet += "  background-color: rgba(12, 16, 26, 191);";
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
   styleSheet += "  font-size: 16px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.8px;";
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
   styleSheet += "  font-size: 12px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.8px;";
   styleSheet += "  color: rgba(220, 226, 242, 220);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QLabel#detailValue {";
   styleSheet += "  font-size: 14px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "  letter-spacing: 0.5px;";
   styleSheet += "  color: rgb(236, 240, 255);";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#viewEasTextButton {";
   styleSheet += "  border: 1px solid rgba(" + accentColor + ", 200);";
   styleSheet += "  background-color: rgba(2, 6, 14, 235);";
   styleSheet += "  color: rgb(248, 251, 255);";
   styleSheet += "  font-size: 16px;";
   styleSheet += "  font-weight: 800;";
   styleSheet += "  letter-spacing: 0.8px;";
   styleSheet += "  padding: 8px;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#closeButton {";
   styleSheet += "  border-radius: 17px;";
   styleSheet += "  border: 1px solid rgba(160, 170, 194, 190);";
   styleSheet += "  background-color: rgba(5, 9, 20, 210);";
   styleSheet += "  color: rgba(235, 240, 255, 230);";
   styleSheet += "  font-size: 18px;";
   styleSheet += "  font-weight: 700;";
   styleSheet += "}";
   styleSheet += "QWidget#WarningBoxWidget QPushButton#closeButton:hover {";
   styleSheet += "  background-color: rgba(" + accentColor + ", 72);";
   styleSheet += "}";

   self_->setStyleSheet(QString::fromStdString(styleSheet));
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
      AddSummaryField("Hail Threat", fields, {"HAIL THREAT"});
      AddSummaryField("Max Hail Size", fields, {"MAX HAIL SIZE"});
      AddSummaryField("Wind Threat", fields, {"WIND THREAT"});
      AddSummaryField("Max Wind Gust", fields, {"MAX WIND GUST"});
      AddSummaryField(
         "Damage", fields, {"THUNDERSTORM DAMAGE THREAT", "DAMAGE THREAT"});
   }
   else if (phenomenon == awips::Phenomenon::Tornado)
   {
      AddSummaryField("Hail Threat", fields, {"HAIL THREAT"});
      AddSummaryField("Max Hail Size", fields, {"MAX HAIL SIZE"});
      AddSummaryField("Wind Threat", fields, {"WIND THREAT"});
      AddSummaryField("Max Wind Gust", fields, {"MAX WIND GUST"});
      AddSummaryField(
         "Damage Threat", fields, {"TORNADO DAMAGE THREAT", "DAMAGE THREAT"});
      AddSummaryField("Tornado Threat", fields, {"TORNADO THREAT"});
   }
}

void WarningBoxWidgetImpl::UpdateCountdown()
{
   if (currentKey_ == types::TextEventKey {})
      return;
   PopulateFromWarning(currentKey_);
   auto messages = textEventManager_->message_list(currentKey_);
   if (messages.empty())
      return;
   auto& segment  = messages.back()->segments().back();
   auto  eventEnd = segment->event_end();
   auto  now      = std::chrono::system_clock::now();
   if (std::chrono::duration_cast<std::chrono::minutes>(eventEnd - now)
          .count() <= 0)
      updateTimer_->stop();
}

#include "warning_box_widget.moc"

} // namespace ui
} // namespace qt
} // namespace scwx
