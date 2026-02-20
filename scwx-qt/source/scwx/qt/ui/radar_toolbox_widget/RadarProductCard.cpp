#include "RadarProductCard.hpp"

#include <QLabel>
#include <QIcon>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyle>

namespace scwx::qt::ui
{

RadarProductCard::RadarProductCard(const QString& title,
                                   const QString& iconPath,
                                   QWidget*       parent) :
    QWidget(parent)
{
   setCursor(Qt::PointingHandCursor);
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins(8, 10, 8, 10);
   layout->setSpacing(6);

   // -------------------------
   // Icon container
   // -------------------------
   iconContainer_ = new QWidget(this);
   iconContainer_->setFixedSize(44, 44);

   auto* iconLayout = new QVBoxLayout(iconContainer_);
   iconLayout->setContentsMargins(4, 4, 4, 4);
   iconLayout->setAlignment(Qt::AlignCenter);

   imageLabel_ = new QLabel(iconContainer_);
   imageLabel_->setFixedSize(36, 36);
   imageLabel_->setAlignment(Qt::AlignCenter);

   iconLayout->addWidget(imageLabel_);

   // -------------------------
   // Load high-DPI icon
   // -------------------------
   QIcon icon(iconPath);
   qreal dpr = devicePixelRatioF();

   QSize pixelSize = QSize(36, 36) * dpr;

   QPixmap source = icon.pixmap(pixelSize);
   source.setDevicePixelRatio(dpr);

   QPixmap rounded(pixelSize);
   rounded.setDevicePixelRatio(dpr);
   rounded.fill(Qt::transparent);

   QPainter painter(&rounded);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.setRenderHint(QPainter::SmoothPixmapTransform);

   QPainterPath path;
   path.addRoundedRect(QRectF(0, 0, 36 * dpr, 36 * dpr), 6 * dpr, 6 * dpr);

   painter.setClipPath(path);
   painter.drawPixmap(0, 0, source);
   painter.end();

   imageLabel_->setPixmap(rounded);

   // -------------------------
   // Title
   // -------------------------
   titleLabel_ = new QLabel(title, this);
   titleLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
   titleLabel_->setWordWrap(true);
   titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   titleLabel_->setMinimumWidth(0);
   titleLabel_->setObjectName("titleLabel");

   layout->addWidget(iconContainer_, 0, Qt::AlignHCenter);
   layout->addWidget(titleLabel_);

   UpdateStyle();
}

QSize RadarProductCard::sizeHint() const
{
   return layout()->sizeHint();
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
   QColor textColor = palette().color(QPalette::WindowText);

   QColor hoverColor(textColor);
   hoverColor.setAlpha(hovered_ ? 30 : 16);

   setStyleSheet(QString("background-color: %1;"
                         "border-radius: 10px;")
                    .arg(hoverColor.name(QColor::HexArgb)));

   if (selected_)
   {
      iconContainer_->setStyleSheet(
         "border: 2px solid #2979FF;"
         "border-radius: 8px;");
      titleLabel_->setStyleSheet(
         "font-weight: 600;"
         "font-size: 12px;"
         "color: #2979FF;");
   }
   else
   {
      iconContainer_->setStyleSheet(
         "border: 2px solid transparent;"
         "border-radius: 8px;");
      titleLabel_->setStyleSheet(QString("font-weight: 600;"
                                         "font-size: 12px;"
                                         "color: %1;")
                                    .arg(textColor.name()));
   }
}

void RadarProductCard::SetSelected(bool selected)
{
   selected_ = selected;
   UpdateStyle();
}

} // namespace scwx::qt::ui
