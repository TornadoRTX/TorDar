#include "RadarProductCard.hpp"

#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QMouseEvent>
#include <QIcon>
#include <QVBoxLayout>
#include <QDebug>
#include <QStyle>

namespace scwx::qt::ui
{

namespace
{
QString GetInterTightBoldFontFamily()
{
   static QString fontFamily {};
   static bool    loaded {false};

   if (!loaded)
   {
      const int fontId =
         QFontDatabase::addApplicationFont(":/res/fonts/InterTight-Bold.ttf");

      if (fontId != -1)
      {
         const QStringList families =
            QFontDatabase::applicationFontFamilies(fontId);

         if (!families.isEmpty())
         {
            fontFamily = families.front();
         }
      }

      loaded = true;
   }

   if (fontFamily.isEmpty())
   {
      fontFamily = "Sans Serif";
   }

   return fontFamily;
}
} // namespace

RadarProductCard::RadarProductCard(const QString& title,
                                   const QString& iconPath,
                                   QWidget*       parent) :
    QWidget(parent)
{
   setCursor(Qt::PointingHandCursor);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   setMinimumWidth(87);
   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins(3, 5, 3, 5);
   layout->setSpacing(12);

   // -------------------------
   // Icon
   // -------------------------
   iconContainer_ = new QWidget(this);
   iconContainer_->setFixedSize(87, 86);
   iconContainer_->setProperty("selected", false);

   QVBoxLayout* iconLayout = new QVBoxLayout(iconContainer_);
   iconLayout->setContentsMargins(3, 3, 3, 3);
   iconLayout->setAlignment(Qt::AlignCenter);

   imageLabel_ = new QLabel(iconContainer_);
   imageLabel_->setFixedSize(80, 78);
   imageLabel_->setAlignment(Qt::AlignCenter);
   imageLabel_->setScaledContents(true);

   iconLayout->addWidget(imageLabel_);

   QIcon icon(iconPath);

   if (icon.isNull())
   {
      qWarning() << "RadarProductCard: failed to load icon:" << iconPath;
   }

   imageLabel_->setPixmap(icon.pixmap(80, 78));

   // -------------------------
   // Title
   // -------------------------
   titleLabel_ = new QLabel(this);
   titleLabel_->setTextFormat(Qt::RichText);
   titleLabel_->setText(QString("<div style='line-height: 6px;'>%1</div>")
                           .arg(title.toHtmlEscaped()));
   titleLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
   titleLabel_->setWordWrap(true);
   titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   titleLabel_->setMinimumWidth(0);
   titleLabel_->setProperty("selected", false);
   QFont titleFont {GetInterTightBoldFontFamily()};
   titleFont.setPixelSize(11);
   titleFont.setWeight(QFont::Bold);
   titleLabel_->setFont(titleFont);

   // -------------------------
   // Layout
   // -------------------------
   layout->addWidget(iconContainer_, 0, Qt::AlignHCenter);
   layout->addWidget(titleLabel_);
   iconContainer_->setObjectName("iconContainer");
   titleLabel_->setObjectName("titleLabel");

   iconContainer_->setStyleSheet(
      "#iconContainer {"
      "  border: 5px solid #f1f1f1;"
      "  border-radius: 6px;"
      "}"
      "#iconContainer[selected='true'] {"
      "  border: 5px solid #2979FF;"
      "}");

   titleLabel_->setStyleSheet(
      "#titleLabel {"
      "  background: transparent;"
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
         "background-color: #1a1a1a;"
         "border-radius: 8px;");
   }
   else
   {
      setStyleSheet(
         "background-color: rgba(255, 255, 255, 0.06);"
         "border-radius: 8px;");
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
