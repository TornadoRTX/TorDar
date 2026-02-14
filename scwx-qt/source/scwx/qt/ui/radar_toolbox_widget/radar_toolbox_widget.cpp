#include "radar_toolbox_widget.hpp"
#include "RadarProductCard.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
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
void RadarToolboxWidget::SetLevel2ProductsWidget(QWidget* widget)
{
   if (widget == nullptr)
   {
      return;
   }

   widget->setParent(scrollContents_);
   productsLayout_->insertWidget(productsLayout_->count() - 1, widget);
}

void RadarToolboxWidget::SetLevel3ProductsWidget(QWidget* widget)
{
   if (widget == nullptr)
   {
      return;
   }

   widget->setParent(scrollContents_);
   productsLayout_->insertWidget(productsLayout_->count() - 1, widget);
}

RadarToolboxWidget::RadarToolboxWidget(QWidget* parent) : QWidget(parent)
{
   rootLayout_ = new QVBoxLayout(this);
   rootLayout_->setContentsMargins(8, 8, 8, 8);
   rootLayout_->setSpacing(8);

   // -------------------------
   // Header
   // -------------------------
   headerContainer_   = new QWidget(this);
   auto* headerLayout = new QHBoxLayout(headerContainer_);
   headerLayout->setContentsMargins(0, 0, 0, 0);
   headerLayout->setSpacing(6);

   titleLabel_ = new QLabel(tr("RADAR TOOLBOX"), headerContainer_);
   titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

   collapseButton_ = new QPushButton("▼", headerContainer_);
   collapseButton_->setFixedSize(24, 24);
   collapseButton_->setFlat(true);

   headerLayout->addWidget(titleLabel_);
   headerLayout->addStretch();
   headerLayout->addWidget(collapseButton_);

   rootLayout_->addWidget(headerContainer_);

   // -------------------------
   // Playback row
   // -------------------------
   playbackRow_         = new QWidget(this);
   auto* playbackLayout = new QHBoxLayout(playbackRow_);
   playbackLayout->setContentsMargins(0, 0, 0, 0);
   playbackLayout->setSpacing(6);

   backBtn_      = new QPushButton(tr("⏮"), playbackRow_);
   playPauseBtn_ = new QPushButton(tr("▶"), playbackRow_);
   forwardBtn_   = new QPushButton(tr("⏭"), playbackRow_);
   stopBtn_      = new QPushButton(tr("⏹"), playbackRow_);

   for (auto* btn : {backBtn_, playPauseBtn_, forwardBtn_, stopBtn_})
   {
      btn->setFixedSize(28, 28);
      btn->setFlat(true);
      playbackLayout->addWidget(btn);
   }

   playbackLayout->addStretch();
   rootLayout_->addWidget(playbackRow_);

   // -------------------------
   // Scroll area (products)
   // -------------------------
   scrollArea_ = new QScrollArea(this);
   scrollArea_->setWidgetResizable(true);
   scrollArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   scrollContents_ = new QWidget(scrollArea_);
   scrollContents_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

   productsLayout_ = new QVBoxLayout(scrollContents_);
   productsLayout_->setSpacing(6);
   productsLayout_->setContentsMargins(0, 0, 0, 0);

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
      {"Enhanced Echo Tops", ":/res/icons/radar/ETT.svg"},
      {"Echo Tops", ":/res/icons/radar/placeholder.svg"},
      {"Storm Relative Velocity", ":/res/icons/radar/SRV-SRM.svg"},
      {"Vertical Integrated Liquid", ":/res/icons/radar/VIL.svg"},
      {"One Hour Precip. Accum.", ":/res/icons/radar/OHPA.svg"},
      {"Storm Total Precip. Accum.", ":/res/icons/radar/STPA.svg"},
      {"Clutter Filter Power Removed", ":/res/icons/radar/placeholder.svg"}};

   for (const auto& product : products)
   {
      productsLayout_->addWidget(
         new RadarProductCard(product.name, product.icon, scrollContents_));
   }

   productsLayout_->addStretch();

   scrollArea_->setWidget(scrollContents_);
   rootLayout_->addWidget(scrollArea_, 1);

   // -------------------------
   // Collapse behavior
   // -------------------------
   connect(collapseButton_,
           &QPushButton::clicked,
           this,
           [this]()
           {
              collapsed_ = !collapsed_;
              scrollArea_->setVisible(!collapsed_);
              // playbackRow should NOT be collapsed.
              // playbackRow_->setVisible(!collapsed_);
              collapseButton_->setText(collapsed_ ? "▲" : "▼");
           });

   // -------------------------
   // Play / Pause toggle
   // -------------------------
   connect(playPauseBtn_,
           &QPushButton::clicked,
           this,
           [this]()
           {
              isPlaying_ = !isPlaying_;
              playPauseBtn_->setText(isPlaying_ ? "⏸" : "▶");
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
              playPauseBtn_->setText("▶");
              // Future: jump to latest frame + re-enable auto-update
           });
}

} // namespace scwx::qt::ui