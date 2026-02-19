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

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <sstream>

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
   setStyleSheet("WarningBoxWidget { background-color: rgba(0, 0, 0, 0.85); }");

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

   // Title: from phenomenon and significance (e.g. "Tornado Warning")
   std::string phenText = awips::GetPhenomenonText(key.phenomenon_);
   std::string sigText  = awips::GetSignificanceText(key.significance_);
   std::string title    = fmt::format("{} {}", phenText, sigText);
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
            QString::fromStdString(fmt::format("AREAS: {}", countiesStr)));
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

   // Build detail rows from the actual message content (labels match the
   // warning)
   std::string        content = message->message_content();
   std::string        line;
   std::istringstream iss(content);
   while (std::getline(iss, line))
   {
      // Trim
      auto start = line.find_first_not_of(" \t\r\n*");
      if (start == std::string::npos)
         continue;
      auto end = line.find_last_not_of(" \t\r\n");
      line     = line.substr(
         start, end == std::string::npos ? std::string::npos : end - start + 1);
      if (line.empty())
         continue;

      // NWS format often has "LABEL...VALUE" or "LABEL: VALUE"
      std::string label, value;
      size_t      sep = line.find("...");
      if (sep != std::string::npos)
      {
         label = line.substr(0, sep);
         value = line.substr(sep + 3);
      }
      else
      {
         sep = line.find(": ");
         if (sep != std::string::npos)
         {
            label = line.substr(0, sep);
            value = line.substr(sep + 2);
         }
         else
         {
            value = line;
            label.clear();
         }
      }

      // Trim label/value
      auto trim = [](std::string& s)
      {
         auto a = s.find_first_not_of(" \t");
         auto b = s.find_last_not_of(" \t");
         if (a == std::string::npos)
            s.clear();
         else
            s = s.substr(
               a, b == std::string::npos ? std::string::npos : b - a + 1);
      };
      trim(label);
      trim(value);
      if (value.empty())
         continue;

      QLabel* labelW = new QLabel(QString::fromStdString(label + ":"));
      labelW->setStyleSheet(
         "color: rgba(255,255,255,0.85); font-weight: bold;");
      QLabel* valueW = new QLabel(QString::fromStdString(value));
      valueW->setStyleSheet("color: white;");
      valueW->setWordWrap(true);

      if (!label.empty())
      {
         QHBoxLayout* row = new QHBoxLayout();
         row->addWidget(labelW, 0);
         row->addWidget(valueW, 1);
         detailsLayout_->addLayout(row);
      }
      else
      {
         detailsLayout_->addWidget(valueW);
      }
   }

   detailsLayout_->addStretch();
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
