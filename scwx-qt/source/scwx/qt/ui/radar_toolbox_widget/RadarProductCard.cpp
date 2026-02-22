#include "RadarProductCard.hpp"

#include <QLabel>
#include <QMouseEvent>
#include <QIcon>
#include <QVBoxLayout>
#include <QDebug>
#include <QStyle>

namespace scwx::qt::ui
{

RadarProductCard::RadarProductCard(const QString& title,
                                   const QString& iconPath,
                                   QWidget*       parent) :
    QWidget(parent)
{
   setCursor(Qt::PointingHandCursor);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   setMinimumWidth(130);
   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins(4, 6, 4, 6);
   layout->setSpacing(10);

   // -------------------------
   // Icon
   // -------------------------
   iconContainer_ = new QWidget(this);
   iconContainer_->setFixedSize(130, 130);
   iconContainer_->setProperty("selected", false);

   QVBoxLayout* iconLayout = new QVBoxLayout(iconContainer_);
   iconLayout->setContentsMargins(5, 5, 5, 5);
   iconLayout->setAlignment(Qt::AlignCenter);

   imageLabel_ = new QLabel(iconContainer_);
   imageLabel_->setFixedSize(120, 120);
   imageLabel_->setAlignment(Qt::AlignCenter);
   imageLabel_->setScaledContents(true);

   iconLayout->addWidget(imageLabel_);

   QIcon icon(iconPath);

   if (icon.isNull())
   {
      qWarning() << "RadarProductCard: failed to load icon:" << iconPath;
   }

   imageLabel_->setPixmap(icon.pixmap(120, 120));

   // -------------------------
   // Title
   // -------------------------
   titleLabel_ = new QLabel(title, this);
   titleLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
   titleLabel_->setWordWrap(true);
   titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   titleLabel_->setMinimumWidth(0);
   titleLabel_->setProperty("selected", false);

   // -------------------------
   // Layout
   // -------------------------
   layout->addWidget(iconContainer_, 0, Qt::AlignHCenter);
   layout->addWidget(titleLabel_);
   iconContainer_->setObjectName("iconContainer");
   titleLabel_->setObjectName("titleLabel");

   iconContainer_->setStyleSheet(
      "#iconContainer {"
      "  border: 2px solid #f1f1f1;"
      "  border-radius: 8px;"
      "}"
      "#iconContainer[selected='true'] {"
      "  border: 2px solid #2979FF;"
      "}");

   titleLabel_->setStyleSheet(
      "#titleLabel {"
      "  background: transparent;"
      "  font-weight: 800;"
      "  font-size: 15px;"
      "  color: white;"
      "}"
      "#titleLabel[selected='true'] {"
      "  color: #2979FF;"
      "}");

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
   if (hovered_)
   {
      setStyleSheet(
         "background-color: rgba(255, 255, 255, 0.06);"
         "border-radius: 10px;");
   }
   else
   {
      setStyleSheet(
         "background-color: transparent;"
         "border-radius: 10px;");
   }
}

void RadarProductCard::SetSelected(bool selected)
{
   selected_ = selected;

   iconContainer_->setProperty("selected", selected_);
   titleLabel_->setProperty("selected", selected_);

   iconContainer_->style()->unpolish(iconContainer_);
   iconContainer_->style()->polish(iconContainer_);

   titleLabel_->style()->unpolish(titleLabel_);
   titleLabel_->style()->polish(titleLabel_);

   iconContainer_->update();
   titleLabel_->update();
}

} // namespace scwx::qt::ui
