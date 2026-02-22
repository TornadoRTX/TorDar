#include "radar_toolbox_widget.hpp"
#include "RadarProductCard.hpp"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <vector>

namespace
{

struct ProductInfo
{
   QString name;
   QString icon;
};

} // namespace

namespace scwx::qt::ui
{

RadarToolboxWidget::RadarToolboxWidget(QWidget* parent) : QWidget(parent)
{
   setObjectName("RadarToolboxFloatingPanel");
   setAttribute(Qt::WA_StyledBackground, true);
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   setMinimumWidth(470);
   setMaximumWidth(680);

   if (parent != nullptr)
   {
      parent->installEventFilter(this);
   }

   rootLayout_ = new QVBoxLayout(this);
   rootLayout_->setContentsMargins(0, 0, 0, 0);
   rootLayout_->setSpacing(0);

   // -------------------------
   // Header
   // -------------------------
   headerContainer_ = new QWidget(this);
   headerContainer_->setObjectName("RadarToolboxHeader");
   auto* headerLayout = new QHBoxLayout(headerContainer_);
   headerLayout->setContentsMargins(16, 8, 12, 8);
   headerLayout->setSpacing(8);

   titleLabel_ = new QLabel(tr("RADAR TOOLBOX"), headerContainer_);
   titleLabel_->setObjectName("RadarToolboxTitle");
   titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

   collapseButton_ = new QPushButton("^", headerContainer_);
   collapseButton_->setObjectName("RadarToolboxCollapseButton");
   collapseButton_->setFixedSize(30, 30);
   collapseButton_->setFlat(true);

   playbackRow_ = new QWidget(this);
   playbackRow_->setObjectName("RadarToolboxPlaybackRow");
   auto* playbackLayout = new QHBoxLayout(playbackRow_);
   playbackLayout->setContentsMargins(0, 0, 0, 0);
   playbackLayout->setSpacing(6);

   backBtn_      = new QPushButton(tr("<"), playbackRow_);
   playPauseBtn_ = new QPushButton(tr(">"), playbackRow_);
   forwardBtn_   = new QPushButton(tr(">"), playbackRow_);
   stopBtn_      = new QPushButton(tr("[]"), playbackRow_);

   backBtn_->setObjectName("RadarToolboxControlButton");
   playPauseBtn_->setObjectName("RadarToolboxControlButton");
   forwardBtn_->setObjectName("RadarToolboxControlButton");
   stopBtn_->setObjectName("RadarToolboxControlButtonStop");

   for (auto* btn : {backBtn_, playPauseBtn_, forwardBtn_, stopBtn_})
   {
      btn->setFixedSize(40, 40);
      btn->setFlat(true);
      playbackLayout->addWidget(btn);
   }

   headerLayout->addWidget(titleLabel_);
   headerLayout->addWidget(collapseButton_, 0, Qt::AlignVCenter);
   headerLayout->addStretch();
   headerLayout->addWidget(playbackRow_, 0, Qt::AlignVCenter);

   rootLayout_->addWidget(headerContainer_);

   headerDivider_ = new QWidget(this);
   headerDivider_->setObjectName("RadarToolboxHeaderDivider");
   headerDivider_->setFixedHeight(2);
   rootLayout_->addWidget(headerDivider_);

   bodyContainer_   = new QWidget(this);
   auto* bodyLayout = new QVBoxLayout(bodyContainer_);
   bodyLayout->setContentsMargins(16, 16, 16, 16);
   bodyLayout->setSpacing(0);

   // -------------------------
   // Scroll area (products)
   // -------------------------
   scrollArea_ = new QScrollArea(bodyContainer_);
   scrollArea_->setFrameShape(QFrame::NoFrame);
   scrollArea_->setWidgetResizable(true);
   scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   scrollArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

   scrollContents_ = new QWidget(scrollArea_);
   scrollContents_->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Preferred);

   productsLayout_ = new QGridLayout(scrollContents_);
   productsLayout_->setSpacing(18);
   productsLayout_->setContentsMargins(0, 0, 0, 0);
   productsLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

   const std::vector<ProductInfo> products = {
      {"Reflectivity", ":/res/icons/radar/reflectivity.svg"},
      {"Velocity", ":/res/icons/radar/velocity.svg"},
      {"Correlation Coefficient", ":/res/icons/radar/cc.svg"},
      {"Differential Reflectivity", ":/res/icons/radar/ZDR.svg"},
      {"Spectrum Width", ":/res/icons/radar/SW.svg"},
      {"Precipitation Type", ":/res/icons/radar/PRT.svg"},
      {"Differential Phase", ":/res/icons/radar/placeholder.svg"},
      {"Specific Differential Phase", ":/res/icons/radar/KDP.svg"},
      {"Hydrometeor Classification", ":/res/icons/radar/HCA.svg"},
      {"Enhanced Echo Tops", ":/res/icons/radar/EET.svg"},
      {"Echo Tops", ":/res/icons/radar/placeholder.svg"},
      {"Storm Relative Velocity", ":/res/icons/radar/SRV-SRM.svg"},
      {"Vertical Integrated Liquid", ":/res/icons/radar/VIL.svg"},
      {"One Hour Precip. Accum.", ":/res/icons/radar/OHPA.svg"},
      {"Storm Total Precip. Accum.", ":/res/icons/radar/STPA.svg"},
      {"Clutter Filter Power Removed", ":/res/icons/radar/placeholder.svg"}};

   for (const auto& product : products)
   {
      auto* card =
         new RadarProductCard(product.name, product.icon, scrollContents_);

      connect(card,
              &RadarProductCard::Clicked,
              this,
              [this, card]()
              {
                 auto cards =
                    scrollContents_->findChildren<RadarProductCard*>();

                 for (auto* c : cards)
                 {
                    c->SetSelected(false);
                 }

                 card->SetSelected(true);
              });

      productsLayout_->addWidget(
         card,
         static_cast<int>(productsLayout_->count() / 3),
         static_cast<int>(productsLayout_->count() % 3));
   }

   scrollArea_->setWidget(scrollContents_);
   bodyLayout->addWidget(scrollArea_);
   rootLayout_->addWidget(bodyContainer_);

   // -------------------------
   // Collapse behavior
   // -------------------------
   connect(collapseButton_,
           &QPushButton::clicked,
           this,
           [this]() { SetCollapsed(!collapsed_); });

   // -------------------------
   // Play / Pause toggle
   // -------------------------
   connect(playPauseBtn_,
           &QPushButton::clicked,
           this,
           [this]()
           {
              isPlaying_ = !isPlaying_;
              playPauseBtn_->setText(isPlaying_ ? "||" : ">");
           });

   // -------------------------
   // Stop / Live (UI reset only)
   // -------------------------
   connect(stopBtn_,
           &QPushButton::clicked,
           this,
           [this]()
           {
              isPlaying_ = false;
              playPauseBtn_->setText(">");
              // Future: jump to latest frame + re-enable auto-update
           });

   setStyleSheet(
      "#RadarToolboxFloatingPanel {"
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
      "     stop:0 #06080d, stop:1 #3f434a);"
      "  border: 4px solid #f2f2f2;"
      "  border-radius: 24px;"
      "}"
      "#RadarToolboxHeader {"
      "  min-height: 56px;"
      "  background: #000000;"
      "  border-top-left-radius: 20px;"
      "  border-top-right-radius: 20px;"
      "}"
      "#RadarToolboxTitle {"
      "  color: #ffffff;"
      "  font-size: 22px;"
      "  font-weight: 900;"
      "  letter-spacing: 1px;"
      "}"
      "#RadarToolboxHeaderDivider {"
      "  background: #ffffff;"
      "}"
      "#RadarToolboxCollapseButton {"
      "  color: #ffffff;"
      "  font-size: 22px;"
      "  border: none;"
      "  background: transparent;"
      "}"
      "#RadarToolboxCollapseButton:hover {"
      "  color: #d9d9d9;"
      "}"
      "#RadarToolboxControlButton,"
      "#RadarToolboxControlButtonStop {"
      "  color: #f2f2f2;"
      "  background: #0e95ff;"
      "  border: 2px solid #0378d4;"
      "  border-radius: 12px;"
      "  font-size: 26px;"
      "  font-weight: 900;"
      "  min-width: 40px;"
      "}"
      "#RadarToolboxControlButton:hover,"
      "#RadarToolboxControlButtonStop:hover {"
      "  background: #25a7ff;"
      "}"
      "#RadarToolboxControlButtonStop {"
      "  color: #dbe9f8;"
      "  font-size: 20px;"
      "}");

   SetCollapsed(false);
}

bool RadarToolboxWidget::eventFilter(QObject* watched, QEvent* event)
{
   if (watched == parentWidget() && event->type() == QEvent::Resize)
   {
      ClampToParent();
      raise();
   }

   return QWidget::eventFilter(watched, event);
}

void RadarToolboxWidget::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);

   if (!positioned_)
   {
      move(20, 20);
      positioned_ = true;
   }

   ClampToParent();
   raise();
}

void RadarToolboxWidget::SetCollapsed(bool collapsed)
{
   collapsed_ = collapsed;
   bodyContainer_->setVisible(!collapsed_);
   headerDivider_->setVisible(!collapsed_);
   collapseButton_->setText(collapsed_ ? "v" : "^");

   if (collapsed_)
   {
      setMaximumHeight(headerContainer_->sizeHint().height() + 8);
   }
   else
   {
      setMaximumHeight(QWIDGETSIZE_MAX);
   }

   adjustSize();
   ClampToParent();
}

void RadarToolboxWidget::ClampToParent()
{
   QWidget* parent = parentWidget();

   if (parent == nullptr)
   {
      return;
   }

   const QRect bounds = parent->rect();
   int         x      = this->x();
   int         y      = this->y();

   x = std::max(0, std::min(x, bounds.width() - width()));
   y = std::max(0, std::min(y, bounds.height() - height()));

   move(x, y);
}

} // namespace scwx::qt::ui
