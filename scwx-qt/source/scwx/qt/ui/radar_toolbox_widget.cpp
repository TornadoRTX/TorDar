#include "ui/radar_toolbox_widget.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace ui
{

RadarToolboxWidget::RadarToolboxWidget(QWidget* parent) : QWidget(parent)
{
   rootLayout_ = new QVBoxLayout(this);
   rootLayout_->setContentsMargins(8, 8, 8, 8);
   rootLayout_->setSpacing(8);

   // Header
   header_ = new QLabel(tr("RADAR TOOLBOX"), this);
   rootLayout_->addWidget(header_);

   // Playback row (placeholder)
   playbackRow_ = new QWidget(this);
   playbackRow_->setFixedHeight(32);
   rootLayout_->addWidget(playbackRow_);

   // Scroll area
   scrollArea_ = new QScrollArea(this);
   scrollArea_->setWidgetResizable(true);

   scrollContents_    = new QWidget(scrollArea_);
   auto* scrollLayout = new QVBoxLayout(scrollContents_);
   scrollLayout->setSpacing(6);

   // Placeholder product buttons
   scrollLayout->addWidget(new QPushButton(tr("Reflectivity"), this));
   scrollLayout->addWidget(new QPushButton(tr("Velocity"), this));
   scrollLayout->addWidget(
      new QPushButton(tr("Correlation Coefficient"), this));
   scrollLayout->addStretch();

   scrollArea_->setWidget(scrollContents_);
   rootLayout_->addWidget(scrollArea_);
}

} // namespace ui
