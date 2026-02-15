#include "RadarProductCard.hpp"

#include <QLabel>
#include <QMouseEvent>
#include <QIcon>
#include <QVBoxLayout>
#include <QDebug>

namespace scwx::qt::ui
{

RadarProductCard::RadarProductCard(const QString& title,
                                   const QString& iconPath,
                                   QWidget*       parent) :
    QWidget(parent)
{
   setCursor(Qt::PointingHandCursor);
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   setMinimumHeight(80);
   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins(8, 10, 8, 10);
   layout->setSpacing(6);

   // -------------------------
   // Icon
   // -------------------------
   imageLabel_ = new QLabel(this);
   imageLabel_->setFixedSize(36, 36);
   imageLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
   imageLabel_->setScaledContents(true);

   QIcon icon(iconPath);

   if (icon.isNull())
   {
      qWarning() << "RadarProductCard: failed to load icon:" << iconPath;
   }

   imageLabel_->setPixmap(icon.pixmap(36, 36));

   // -------------------------
   // Title
   // -------------------------
   titleLabel_ = new QLabel(title, this);
   titleLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
   titleLabel_->setWordWrap(true);
   titleLabel_->setStyleSheet("font-weight: 600; font-size: 12px;");

   // -------------------------
   // Layout
   // -------------------------
   layout->addWidget(imageLabel_, 0, Qt::AlignHCenter);
   layout->addWidget(titleLabel_, 0, Qt::AlignHCenter);
   layout->addStretch();

   UpdateStyle();
}

void RadarProductCard::mousePressEvent(QMouseEvent* event)
{
   if (event->button() == Qt::LeftButton)
   {
      Clicked();
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
         "border-radius: 10px;");
   }
   else
   {
      setStyleSheet(
         "background-color: rgba(255, 255, 255, 0.03);"
         "border-radius: 10px;");
   }
}

} // namespace scwx::qt::ui
