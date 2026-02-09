#include "radar_toolbox_widget.hpp"
#include "RadarProductCard.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

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

   scrollContents_ = new QWidget(scrollArea_);

   productsLayout_ = new QVBoxLayout(scrollContents_);
   productsLayout_->setSpacing(6);
   productsLayout_->setContentsMargins(0, 0, 0, 0);
   productsLayout_->addWidget(new RadarProductCard(
      "Reflectivity", ":/res/icons/radar/reflectivity.png", scrollContents_));

   productsLayout_->addWidget(new RadarProductCard(
      "Velocity", ":/res/icons/radar/velocity.png", scrollContents_));

   productsLayout_->addWidget(
      new RadarProductCard("Correlation Coefficient",
                           ":/res/icons/radar/correlation.png",
                           scrollContents_));

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