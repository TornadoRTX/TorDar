#include "radar_toolbox_widget.hpp"
#include "RadarProductCard.hpp"

#include <QFrame>
#include <QFontDatabase>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
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
   static bool gothicLoaded = false;

   if (!gothicLoaded)
   {
      QFontDatabase::addApplicationFont(":/res/fonts/Gothic-No.13-Regular.otf");
      gothicLoaded = true;
   }

   setObjectName("RadarToolboxFloatingPanel");
   setAttribute(Qt::WA_StyledBackground, true);
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   setMinimumWidth(364);
   setMaximumWidth(364);

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
   headerContainer_->installEventFilter(this);
   auto* headerLayout = new QHBoxLayout(headerContainer_);
   headerLayout->setContentsMargins(8, 4, 6, 4);
   headerLayout->setSpacing(4);

   titleLabel_ = new QLabel(tr("RADAR TOOLBOX"), headerContainer_);
   titleLabel_->setObjectName("RadarToolboxTitle");
   titleLabel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   titleLabel_->installEventFilter(this);

   collapseButton_ = new QPushButton("^", headerContainer_);
   collapseButton_->setObjectName("RadarToolboxCollapseButton");
   collapseButton_->setFixedSize(15, 15);
   collapseButton_->setFlat(true);

   playbackRow_ = new QWidget(this);
   playbackRow_->setObjectName("RadarToolboxPlaybackRow");
   playbackRow_->installEventFilter(this);
   auto* playbackLayout = new QHBoxLayout(playbackRow_);
   playbackLayout->setContentsMargins(0, 0, 0, 0);
   playbackLayout->setSpacing(3);

   backBtn_      = new QPushButton(tr("<"), playbackRow_);
   playPauseBtn_ = new QPushButton(tr(">"), playbackRow_);
   forwardBtn_   = new QPushButton(tr(">"), playbackRow_);
   stopBtn_      = new QPushButton(tr("[]"), playbackRow_);

   backBtn_->setObjectName("RadarToolboxControlButton");
   playPauseBtn_->setObjectName("RadarToolboxControlButton");
   forwardBtn_->setObjectName("RadarToolboxControlButton");
   stopBtn_->setObjectName("RadarToolboxControlButtonStop");

   playPauseBtn_->setText("");
   playPauseBtn_->setIcon(QIcon(":/res/icons/play_button.svg"));
   playPauseBtn_->setIconSize(QSize(11, 11));

   stopBtn_->setText("");
   stopBtn_->setIcon(QIcon(":/res/icons/Square.svg"));
   stopBtn_->setIconSize(QSize(9, 9));

   for (auto* btn : {backBtn_, playPauseBtn_, forwardBtn_, stopBtn_})
   {
      btn->setFixedSize(20, 20);
      btn->setFlat(true);
      playbackLayout->addWidget(btn);
   }

   headerLayout->addWidget(titleLabel_);
   headerLayout->addStretch(1);
   headerLayout->addWidget(collapseButton_, 0, Qt::AlignVCenter);
   headerLayout->addStretch(1);
   headerLayout->addWidget(playbackRow_, 0, Qt::AlignVCenter);

   rootLayout_->addWidget(headerContainer_);

   headerDivider_ = new QWidget(this);
   headerDivider_->setObjectName("RadarToolboxHeaderDivider");
   headerDivider_->setFixedHeight(1);
   rootLayout_->addWidget(headerDivider_);

   collapsedPeek_ = new QWidget(this);
   collapsedPeek_->setObjectName("RadarToolboxCollapsedPeek");
   collapsedPeek_->setFixedHeight(3);
   rootLayout_->addWidget(collapsedPeek_);

   bodyContainer_   = new QWidget(this);
   auto* bodyLayout = new QVBoxLayout(bodyContainer_);
   bodyLayout->setContentsMargins(8, 8, 8, 8);
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
   productsLayout_->setHorizontalSpacing(61);
   productsLayout_->setVerticalSpacing(12);
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
              playPauseBtn_->setIcon(QIcon(
                 isPlaying_ ? ":/res/icons/font-awesome-6/pause-solid.svg" :
                              ":/res/icons/play_button.svg"));
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
              playPauseBtn_->setIcon(QIcon(":/res/icons/play_button.svg"));
              // Future: jump to latest frame + re-enable auto-update
           });

   setStyleSheet(
      "#RadarToolboxFloatingPanel {"
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
      "     stop:0rgb(0, 0, 0), stop:1rgb(0, 0, 0));"
      "  border: 3px solid #f2f2f2;"
      "  border-radius: 12px;"
      "}"
      "#RadarToolboxHeader {"
      "  min-height: 28px;"
      "  background: #000000;"
      "  border-top-left-radius: 10px;"
      "  border-top-right-radius: 10px;"
      "}"
      "#RadarToolboxTitle {"
      "  font-family: 'Gothic No.13 Regular', 'Gothic-No.13-Regular';"
      "  color: #ffffff;"
      "  font-size: 18px;"
      "  font-weight: 900;"
      "  letter-spacing: 1px;"
      "}"
      "#RadarToolboxHeaderDivider {"
      "  background: #ffffff;"
      "}"
      "#RadarToolboxCollapsedPeek {"
      "  background:rgb(0, 0, 0);"
      "}"
      "#RadarToolboxCollapseButton {"
      "  font-family: 'Gothic No.13 Regular', 'Gothic-No.13-Regular';"
      "  color: #ffffff;"
      "  font-size: 11px;"
      "  border: none;"
      "  background: #000000;"
      "}"
      "#RadarToolboxCollapseButton:hover {"
      "  color: #d9d9d9;"
      "}"
      "#RadarToolboxControlButton,"
      "#RadarToolboxControlButtonStop {"
      "  font-family: 'Gothic No.13 Regular', 'Gothic-No.13-Regular';"
      "  color: #f2f2f2;"
      "  background: #0e95ff;"
      "  border: 1px solid #0378d4;"
      "  border-radius: 6px;"
      "  font-size: 13px;"
      "  font-weight: 900;"
      "  min-width: 20px;"
      "}"
      "#RadarToolboxControlButton:hover,"
      "#RadarToolboxControlButtonStop:hover {"
      "  background: #25a7ff;"
      "}"
      "#RadarToolboxControlButtonStop {"
      "  color: #dbe9f8;"
      "  font-size: 10px;"
      "}");

   SetCollapsed(false);
}

bool RadarToolboxWidget::eventFilter(QObject* watched, QEvent* event)
{
   if ((watched == headerContainer_ || watched == titleLabel_ ||
        watched == playbackRow_) &&
       event->type() == QEvent::MouseButtonPress)
   {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);

      if (mouseEvent->button() == Qt::LeftButton)
      {
         SetCollapsed(!collapsed_);
         return true;
      }
   }

   if (watched == parentWidget() && event->type() == QEvent::Resize)
   {
      MoveToBottomLeft();
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
      MoveToBottomLeft();
      positioned_ = true;
   }

   ClampToParent();
   raise();
}

void RadarToolboxWidget::SetCollapsed(bool collapsed)
{
   collapsed_ = collapsed;
   playbackRow_->setVisible(true);
   bodyContainer_->setVisible(!collapsed_);
   headerDivider_->setVisible(true);
   collapsedPeek_->setVisible(collapsed_);
   collapseButton_->setText(collapsed_ ? "^" : "v");

   if (collapsed_)
   {
      setMinimumHeight(0);
      setMaximumHeight(QWIDGETSIZE_MAX);
      adjustSize();
      setFixedHeight(sizeHint().height());
   }
   else
   {
      setFixedHeight(176);
   }

   adjustSize();
   MoveToBottomLeft();
   ClampToParent();
}

void RadarToolboxWidget::MoveToBottomLeft()
{
   QWidget* parent = parentWidget();

   if (parent == nullptr)
   {
      return;
   }

   const int x = 10;
   const int y = std::max(0, parent->height() - height() - 10);

   move(x, y);
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
