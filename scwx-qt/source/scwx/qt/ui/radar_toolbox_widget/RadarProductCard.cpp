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
   imageLabel_->setObjectName("iconImage");
   imageLabel_->setProperty("selected", false);

   imageLabel_->setStyleSheet(
      "#iconImage { border-radius: 6px; }"
      "#iconImage[selected='true'] { border: 2px solid #2979FF; }");

   iconLayout->addWidget(imageLabel_);

   // Load icon
   QIcon icon(iconPath);
   QPixmap pixmap = icon.pixmap(QSize(36, 36), QIcon::Normal, QIcon::Off);

   QPixmap rounded(36, 36);
   rounded.fill(Qt::transparent);

   QPainter painter(&rounded);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

   QPainterPath path;
   path.addRoundedRect(0, 0, 36, 36, 6, 6);
   painter.setClipPath(path);
   painter.drawPixmap(0, 0, pixmap);
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
   titleLabel_->setProperty("selected", false);

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
   const QColor textColor = palette().color(QPalette::WindowText);

   // Hover background derived from theme text color
   QColor hoverColor(textColor);
   hoverColor.setAlpha(hovered_ ? 30 : 16);

   titleLabel_->setStyleSheet(QString(
      "#titleLabel {"
      "  background: transparent;"
      "  font-weight: 600;"
      "  font-size: 12px;"
      "  color: %1;"
      "}"
      "#titleLabel[selected='true'] {"
      "  color: #2979FF;"
      "}")
      .arg(textColor.name()));

   setStyleSheet(QString(
      "background-color: %1;"
      "border-radius: 10px;")
      .arg(hoverColor.name(QColor::HexArgb)));
}

void RadarProductCard::SetSelected(bool selected)
{
   selected_ = selected;

   titleLabel_->setProperty("selected", selected_);
   imageLabel_->setProperty("selected", selected_);

   titleLabel_->style()->unpolish(titleLabel_);
   titleLabel_->style()->polish(titleLabel_);

   imageLabel_->style()->unpolish(imageLabel_);
   imageLabel_->style()->polish(imageLabel_);

   titleLabel_->update();
   imageLabel_->update();
}

} // namespace scwx::qt::ui
