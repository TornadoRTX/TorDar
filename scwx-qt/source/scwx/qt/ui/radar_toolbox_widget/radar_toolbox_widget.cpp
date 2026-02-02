#include "radar_toolbox_widget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace scwx::qt::ui
{

RadarToolboxWidget::RadarToolboxWidget(QWidget* parent) : QWidget(parent)
{
   rootLayout_ = new QVBoxLayout(this);
   rootLayout_->setContentsMargins(8, 8, 8, 8);
   rootLayout_->setSpacing(8);

   // Header container
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

   // Playback row
   playbackRow_         = new QWidget(this);
   auto* playbackLayout = new QHBoxLayout(playbackRow_);
   playbackLayout->setContentsMargins(0, 0, 0, 0);
   playbackLayout->setSpacing(6);

   // Buttons (placeholders for now)
   auto* backBtn  = new QPushButton(tr("⏮"), playbackRow_);
   auto* playBtn  = new QPushButton(tr("▶"), playbackRow_);
   auto* pauseBtn = new QPushButton(tr("⏸"), playbackRow_);
   auto* fwdBtn   = new QPushButton(tr("⏭"), playbackRow_);

   for (auto* btn : {backBtn, playBtn, pauseBtn, fwdBtn})
   {
      btn->setFixedSize(28, 28);
      playbackLayout->addWidget(btn);
   }

   playbackLayout->addStretch();
   rootLayout_->addWidget(playbackRow_);

   // Scroll area
   scrollArea_ = new QScrollArea(this);
   scrollArea_->setWidgetResizable(true);

   scrollContents_    = new QWidget(scrollArea_);
   auto* scrollLayout = new QVBoxLayout(scrollContents_);
   scrollLayout->setSpacing(6);

   scrollLayout->addWidget(new QPushButton(tr("Reflectivity"), this));
   scrollLayout->addWidget(new QPushButton(tr("Velocity"), this));
   scrollLayout->addWidget(
      new QPushButton(tr("Correlation Coefficient"), this));
   scrollLayout->addStretch();

   scrollArea_->setWidget(scrollContents_);
   rootLayout_->addWidget(scrollArea_);

   // Collapse behavior (no animation yet)
   connect(collapseButton_,
           &QPushButton::clicked,
           this,
           [this]()
           {
              collapsed_ = !collapsed_;
              scrollArea_->setVisible(!collapsed_);
              collapseButton_->setText(collapsed_ ? "▲" : "▼");
           });
}

} // namespace scwx::qt::ui
