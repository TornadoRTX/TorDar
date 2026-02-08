#include "RadarProductCard.hpp"

#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

namespace scwx::qt::ui
{

RadarProductCard::RadarProductCard(const QString& title,
                                   const QString& subtitle,
                                   QWidget*       parent) :
    QWidget(parent)
{
   setCursor(Qt::PointingHandCursor);

   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins(10, 8, 10, 8);
   layout->setSpacing(2);

   titleLabel_ = new QLabel(title, this);
   titleLabel_->setStyleSheet("font-weight: 600;");

   layout->addWidget(titleLabel_);

   if (!subtitle.isEmpty())
   {
      subtitleLabel_ = new QLabel(subtitle, this);
      subtitleLabel_->setStyleSheet("color: #aaa; font-size: 11px;");
      layout->addWidget(subtitleLabel_);
   }
   else
   {
      subtitleLabel_ = nullptr;
   }

   UpdateStyle();
}

void RadarProductCard::mousePressEvent(QMouseEvent* event)
{
   if (event->button() == Qt::LeftButton)
   {
      emit Clicked();
   }

   QWidget::mousePressEvent(event);
}

void RadarProductCard::enterEvent(QEnterEvent*)
{
   hovered_ = true;
   UpdateStyle();
}

void RadarProductCard::leaveEvent(QEvent*)
{
   hovered_ = false;
   UpdateStyle();
}

void RadarProductCard::UpdateStyle()
{
   if (hovered_)
   {
      setStyleSheet(
         "background-color: rgba(255, 255, 255, 0.08);"
         "border-radius: 6px;");
   }
   else
   {
      setStyleSheet(
         "background-color: rgba(255, 255, 255, 0.04);"
         "border-radius: 6px;");
   }
}

} // namespace scwx::qt::ui
