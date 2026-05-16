#include "radar_toolbox_widget.hpp"
#include "RadarProductCard.hpp"

#include <QAction>
#include <QFrame>
#include <QFont>
#include <QFontDatabase>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <optional>
#include <vector>

namespace
{

struct ProductInfo
{
   QString                                            name;
   QString                                            icon;
   std::optional<scwx::common::Level2Product>         level2Product {};
   std::optional<scwx::common::Level3ProductCategory> level3Category {};
};

void StyleProductMenu(QMenu* menu, const QString& oswaldBoldFontFamily)
{
   QFont menuFont {oswaldBoldFontFamily};
   menuFont.setPixelSize(18);
   menuFont.setWeight(QFont::Bold);
   menu->setFont(menuFont);

   menu->setStyleSheet(
      "QMenu {"
      "  background: #000000;"
      "  border: 2px solid #f2f2f2;"
      "  border-radius: 12px;"
      "  padding: 8px;"
      "}"
      "QMenu::item {"
      "  background: #9c9c9c;"
      "  color: #f2f2f2;"
      "  border-radius: 10px;"
      "  padding: 6px 18px;"
      "  margin: 4px 0px;"
      "}"
      "QMenu::item:selected {"
      "  background: #1f2ef9;"
      "  color: #ffffff;"
      "}"
      "QMenu::item:disabled {"
      "  background: #525252;"
      "  color: #9a9a9a;"
      "}"
      "QMenu::right-arrow {"
      "  width: 10px;"
      "  height: 10px;"
      "}");
}

QString GetOswaldBoldFontFamily()
{
   static QString fontFamily {};
   static bool    loaded {false};

   if (!loaded)
   {
      const int fontId =
         QFontDatabase::addApplicationFont(":/res/fonts/Oswald-Bold.ttf");

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

namespace scwx::qt::ui
{

RadarToolboxWidget::RadarToolboxWidget(QWidget* parent) : QWidget(parent)
{
   const QString oswaldBoldFontFamily = GetOswaldBoldFontFamily();

   setObjectName("RadarToolboxFloatingPanel");
   setAttribute(Qt::WA_StyledBackground, true);
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   setMinimumWidth(546);
   setMaximumWidth(546);

   if (parent != nullptr)
   {
      parent->installEventFilter(this);
   }

   rootLayout_ = new QVBoxLayout(this);
   rootLayout_->setContentsMargins(2, 2, 2, 2);
   rootLayout_->setSpacing(0);

   // -------------------------
   // Header
   // -------------------------
   headerContainer_ = new QWidget(this);
   headerContainer_->setObjectName("RadarToolboxHeader");
   headerContainer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   headerContainer_->setFixedHeight(42);
   headerContainer_->installEventFilter(this);
   auto* headerLayout = new QHBoxLayout(headerContainer_);
   headerLayout->setContentsMargins(12, 6, 9, 6);
   headerLayout->setSpacing(6);

   titleLabel_ = new QLabel(tr("RADAR TOOLBOX"), headerContainer_);
   titleLabel_->setObjectName("RadarToolboxTitle");
   titleLabel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   titleLabel_->installEventFilter(this);
   QFont titleFont {oswaldBoldFontFamily};
   titleFont.setPixelSize(27);
   titleFont.setWeight(QFont::Bold);
   titleLabel_->setFont(titleFont);

   collapseButton_ = new QPushButton("^", headerContainer_);
   collapseButton_->setObjectName("RadarToolboxCollapseButton");
   collapseButton_->setFixedSize(23, 23);
   collapseButton_->setFlat(true);
   QFont collapseFont {oswaldBoldFontFamily};
   collapseFont.setPixelSize(17);
   collapseFont.setWeight(QFont::Bold);
   collapseButton_->setFont(collapseFont);

   playbackRow_ = new QWidget(this);
   playbackRow_->setObjectName("RadarToolboxPlaybackRow");
   playbackRow_->installEventFilter(this);
   auto* playbackLayout = new QHBoxLayout(playbackRow_);
   playbackLayout->setContentsMargins(0, 0, 0, 0);
   playbackLayout->setSpacing(5);

   backBtn_      = new QPushButton(tr("<"), playbackRow_);
   playPauseBtn_ = new QPushButton(tr(">"), playbackRow_);
   forwardBtn_   = new QPushButton(tr(">"), playbackRow_);
   stopBtn_      = new QPushButton(tr("[]"), playbackRow_);

   backBtn_->setObjectName("RadarToolboxControlButton");
   playPauseBtn_->setObjectName("RadarToolboxControlButton");
   forwardBtn_->setObjectName("RadarToolboxControlButton");
   stopBtn_->setObjectName("RadarToolboxControlButtonStop");

   playPauseBtn_->setText("");
   playPauseBtn_->setIcon(QIcon(":/res/icons/play_button.svg"));
   playPauseBtn_->setIconSize(QSize(17, 17));

   stopBtn_->setText("");
   stopBtn_->setIcon(QIcon(":/res/icons/Square.svg"));
   stopBtn_->setIconSize(QSize(14, 14));

   QFont playbackFont {oswaldBoldFontFamily};
   playbackFont.setPixelSize(20);
   playbackFont.setWeight(QFont::Bold);
   backBtn_->setFont(playbackFont);
   forwardBtn_->setFont(playbackFont);

   for (auto* btn : {backBtn_, playPauseBtn_, forwardBtn_, stopBtn_})
   {
      btn->setFixedSize(30, 30);
      btn->setFlat(true);
      playbackLayout->addWidget(btn);
   }

   auto* centerToggleContainer = new QWidget(headerContainer_);
   auto* centerToggleLayout    = new QHBoxLayout(centerToggleContainer);
   centerToggleLayout->setContentsMargins(0, 0, 0, 0);
   centerToggleLayout->setSpacing(0);
   centerToggleLayout->addStretch(1);
   centerToggleLayout->addWidget(collapseButton_, 0, Qt::AlignVCenter);
   centerToggleLayout->addStretch(1);
   centerToggleContainer->setSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Preferred);

   headerLayout->addWidget(titleLabel_, 0, Qt::AlignVCenter);
   headerLayout->addWidget(centerToggleContainer, 1);
   headerLayout->addWidget(playbackRow_, 0, Qt::AlignVCenter);

   rootLayout_->addWidget(headerContainer_);

   headerDivider_ = new QWidget(this);
   headerDivider_->setObjectName("RadarToolboxHeaderDivider");
   headerDivider_->setFixedHeight(2);
   rootLayout_->addWidget(headerDivider_);

   collapsedPeek_ = new QWidget(this);
   collapsedPeek_->setObjectName("RadarToolboxCollapsedPeek");
   collapsedPeek_->setFixedHeight(2);
   rootLayout_->addWidget(collapsedPeek_);

   bodyContainer_ = new QWidget(this);
   bodyContainer_->setObjectName("RadarToolboxBody");
   bodyContainer_->setAttribute(Qt::WA_StyledBackground, true);
   auto* bodyLayout = new QVBoxLayout(bodyContainer_);
   bodyLayout->setContentsMargins(12, 12, 12, 12);
   bodyLayout->setSpacing(0);

   // -------------------------
   // Scroll area (products)
   // -------------------------
   scrollArea_ = new QScrollArea(bodyContainer_);
   scrollArea_->setObjectName("RadarToolboxScrollArea");
   scrollArea_->setFrameShape(QFrame::NoFrame);
   scrollArea_->setWidgetResizable(true);
   scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   scrollArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

   scrollContents_ = new QWidget(scrollArea_);
   scrollContents_->setObjectName("RadarToolboxScrollContents");
   scrollContents_->setAttribute(Qt::WA_StyledBackground, true);
   scrollContents_->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Preferred);

   productsLayout_ = new QGridLayout(scrollContents_);
   productsLayout_->setHorizontalSpacing(92);
   productsLayout_->setVerticalSpacing(18);
   productsLayout_->setContentsMargins(0, 0, 0, 0);
   productsLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

   const std::vector<ProductInfo> products = {
      {"Reflectivity",
       ":/res/icons/radar/reflectivity.svg",
       common::Level2Product::Reflectivity,
       common::Level3ProductCategory::Reflectivity},
      {"Velocity",
       ":/res/icons/radar/velocity.svg",
       common::Level2Product::Velocity,
       common::Level3ProductCategory::Velocity},
      {"Correlation Coefficient",
       ":/res/icons/radar/cc.svg",
       common::Level2Product::CorrelationCoefficient,
       common::Level3ProductCategory::CorrelationCoefficient},
      {"Differential Reflectivity",
       ":/res/icons/radar/ZDR.svg",
       common::Level2Product::DifferentialReflectivity,
       common::Level3ProductCategory::DifferentialReflectivity},
      {"Spectrum Width",
       ":/res/icons/radar/SW.svg",
       common::Level2Product::SpectrumWidth,
       common::Level3ProductCategory::SpectrumWidth},
      {"Precipitation Type",
       ":/res/icons/radar/PRT.svg",
       std::nullopt,
       common::Level3ProductCategory::PrecipitationAccumulation},
      {"Differential Phase",
       ":/res/icons/radar/placeholder.svg",
       common::Level2Product::DifferentialPhase,
       common::Level3ProductCategory::SpecificDifferentialPhase},
      {"Specific Differential Phase",
       ":/res/icons/radar/KDP.svg",
       std::nullopt,
       common::Level3ProductCategory::SpecificDifferentialPhase},
      {"Hydrometeor Classification",
       ":/res/icons/radar/HCA.svg",
       std::nullopt,
       common::Level3ProductCategory::HydrometeorClassification},
      {"Enhanced Echo Tops",
       ":/res/icons/radar/EET.svg",
       std::nullopt,
       common::Level3ProductCategory::EchoTops},
      {"Echo Tops",
       ":/res/icons/radar/placeholder.svg",
       std::nullopt,
       common::Level3ProductCategory::EchoTops},
      {"Storm Relative Velocity",
       ":/res/icons/radar/SRV-SRM.svg",
       std::nullopt,
       common::Level3ProductCategory::StormRelativeVelocity},
      {"Vertical Integrated Liquid",
       ":/res/icons/radar/VIL.svg",
       std::nullopt,
       common::Level3ProductCategory::VerticallyIntegratedLiquid},
      {"One Hour Precip. Accum.",
       ":/res/icons/radar/OHPA.svg",
       std::nullopt,
       common::Level3ProductCategory::PrecipitationAccumulation},
      {"Storm Total Precip. Accum.",
       ":/res/icons/radar/STPA.svg",
       std::nullopt,
       common::Level3ProductCategory::PrecipitationAccumulation},
      {"Clutter Filter Power Removed",
       ":/res/icons/radar/placeholder.svg",
       common::Level2Product::ClutterFilterPowerRemoved,
       std::nullopt}};

   for (const auto& product : products)
   {
      auto* card =
         new RadarProductCard(product.name, product.icon, scrollContents_);

      connect(card,
              &RadarProductCard::Clicked,
              this,
              [this, card]() { SelectProductCard(card); });

      connect(
         card,
         &RadarProductCard::RightClicked,
         this,
         [this, card, product, oswaldBoldFontFamily](const QPoint& globalPos)
         {
            SelectProductCard(card);

            QMenu menu(this);
            StyleProductMenu(&menu, oswaldBoldFontFamily);

            QAction* level2Action = menu.addAction(tr("LEVEL 2 DOPPLER RADAR"));

            if (product.level2Product.has_value())
            {
               const std::string level2Name =
                  common::GetLevel2Name(product.level2Product.value());

               level2Action->setData(
                  QString("L2|%1").arg(QString::fromStdString(level2Name)));
            }
            else
            {
               level2Action->setEnabled(false);
            }

            QMenu* level3Menu = menu.addMenu(tr("LEVEL 3 DOPPLER RADAR"));
            StyleProductMenu(level3Menu, oswaldBoldFontFamily);

            bool hasLevel3Products = false;

            if (product.level3Category.has_value())
            {
               const auto& level3Products = common::GetLevel3ProductsByCategory(
                  product.level3Category.value());

               for (const auto& level3Product : level3Products)
               {
                  const auto& awipsIds =
                     common::GetLevel3AwipsIdsByProduct(level3Product);

                  if (awipsIds.empty())
                  {
                     continue;
                  }

                  const QString actionText =
                     QString::fromStdString(
                        common::GetLevel3ProductDescription(level3Product))
                        .toUpper();

                  QAction* level3Action = level3Menu->addAction(actionText);
                  level3Action->setData(QString("L3|%1").arg(
                     QString::fromStdString(awipsIds.front())));
                  hasLevel3Products = true;
               }
            }

            if (!hasLevel3Products)
            {
               level3Menu->setEnabled(false);
            }

            QAction* selectedAction = menu.exec(globalPos);

            if (selectedAction == nullptr)
            {
               return;
            }

            const QString actionData = selectedAction->data().toString();
            const auto    parts      = actionData.split("|");

            if (parts.size() != 2)
            {
               return;
            }

            const std::string productName = parts.at(1).toStdString();

            if (parts.at(0) == "L2")
            {
               Q_EMIT RadarProductSelected(
                  common::RadarProductGroup::Level2, productName, 0);
            }
            else if (parts.at(0) == "L3")
            {
               Q_EMIT RadarProductSelected(
                  common::RadarProductGroup::Level3, productName, 0);
            }
         });

      productsLayout_->addWidget(
         card,
         static_cast<int>(productsLayout_->count() / 3),
         static_cast<int>(productsLayout_->count() % 3));
   }

   scrollArea_->setWidget(scrollContents_);
   bodyLayout->addWidget(scrollArea_);
   rootLayout_->addWidget(bodyContainer_);

   // -------------------------
   // Collapse behavior
   // -------------------------
   connect(collapseButton_,
           &QPushButton::clicked,
           this,
           [this]() { SetCollapsed(!collapsed_); });

   // -------------------------
   // Play / Pause toggle
   // -------------------------
   connect(playPauseBtn_,
           &QPushButton::clicked,
           this,
           [this]()
           {
              isPlaying_ = !isPlaying_;
              playPauseBtn_->setIcon(QIcon(
                 isPlaying_ ? ":/res/icons/font-awesome-6/pause-solid.svg" :
                              ":/res/icons/play_button.svg"));
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
              playPauseBtn_->setIcon(QIcon(":/res/icons/play_button.svg"));
              // Future: jump to latest frame + re-enable auto-update
           });

   setStyleSheet(
      "#RadarToolboxFloatingPanel {"
      "  background: #000000;"
      "  border: 2px solid #f2f2f2;"
      "  border-radius: 18px;"
      "}"
      "#RadarToolboxFloatingPanel[collapsed=\"true\"] {"
      "  border-bottom: none;"
      "  border-bottom-left-radius: 0px;"
      "  border-bottom-right-radius: 0px;"
      "}"
      "#RadarToolboxHeader {"
      "  min-height: 42px;"
      "  background: #000000;"
      "  border-top-left-radius: 15px;"
      "  border-top-right-radius: 15px;"
      "}"
      "#RadarToolboxTitle {"
      "  font-family: 'Oswald';"
      "  color: #ffffff;"
      "  font-size: 27px;"
      "  font-weight: 900;"
      "  letter-spacing: 1px;"
      "}"
      "#RadarToolboxHeaderDivider {"
      "  background: #ffffff;"
      "}"
      "#RadarToolboxBody {"
      "  background: #000000;"
      "  border-bottom-left-radius: 15px;"
      "  border-bottom-right-radius: 15px;"
      "}"
      "#RadarToolboxScrollArea {"
      "  background: #000000;"
      "  border-bottom-left-radius: 15px;"
      "  border-bottom-right-radius: 15px;"
      "}"
      "#RadarToolboxScrollContents {"
      "  background: #000000;"
      "  border-bottom-left-radius: 15px;"
      "  border-bottom-right-radius: 15px;"
      "}"
      "#RadarToolboxCollapsedPeek {"
      "  background: #000000;"
      "}"
      "#RadarToolboxCollapseButton {"
      "  color: #ffffff;"
      "  border: none;"
      "  background: #000000;"
      "}"
      "#RadarToolboxCollapseButton:hover {"
      "  color: #d9d9d9;"
      "}"
      "#RadarToolboxControlButton,"
      "#RadarToolboxControlButtonStop {"
      "  color: #f2f2f2;"
      "  background: #0e95ff;"
      "  border: 2px solid #0378d4;"
      "  border-radius: 9px;"
      "  font-weight: 900;"
      "  min-width: 30px;"
      "}"
      "#RadarToolboxControlButton:hover,"
      "#RadarToolboxControlButtonStop:hover {"
      "  background: #25a7ff;"
      "}"
      "#RadarToolboxControlButtonStop {"
      "  color: #dbe9f8;"
      "}");

   SetCollapsed(false);
}

void RadarToolboxWidget::SelectProductCard(RadarProductCard* selectedCard)
{
   auto cards = scrollContents_->findChildren<RadarProductCard*>();

   for (auto* card : cards)
   {
      card->SetSelected(false);
   }

   if (selectedCard != nullptr)
   {
      selectedCard->SetSelected(true);
   }
}

bool RadarToolboxWidget::eventFilter(QObject* watched, QEvent* event)
{
   if ((watched == headerContainer_ || watched == titleLabel_ ||
        watched == playbackRow_) &&
       event->type() == QEvent::MouseButtonPress)
   {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);

      if (mouseEvent->button() == Qt::LeftButton)
      {
         SetCollapsed(!collapsed_);
         return true;
      }
   }

   if (watched == parentWidget() && event->type() == QEvent::Resize)
   {
      MoveToBottomLeft();
      ClampToParent();
      raise();
   }

   return QWidget::eventFilter(watched, event);
}

void RadarToolboxWidget::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);

   if (!positioned_)
   {
      MoveToBottomLeft();
      positioned_ = true;
   }

   ClampToParent();
   raise();
}

void RadarToolboxWidget::SetCollapsed(bool collapsed)
{
   collapsed_ = collapsed;
   playbackRow_->setVisible(true);
   bodyContainer_->setVisible(!collapsed_);
   headerDivider_->setVisible(true);
   collapsedPeek_->setVisible(collapsed_);
   collapseButton_->setText(collapsed_ ? "^" : "v");

   if (collapsed_)
   {
      setMinimumHeight(0);
      setMaximumHeight(QWIDGETSIZE_MAX);
      constexpr int kExpandedHeight = 264;
      constexpr int kBottomInset    = 16;
      const int lineTop = headerContainer_->height() + headerDivider_->height();
      const int frameExtra = rootLayout_->contentsMargins().top() +
                             rootLayout_->contentsMargins().bottom();

      int targetY = y();
      int floorY  = y() + lineTop + collapsedPeek_->height();

      if (parentWidget() != nullptr)
      {
         const int parentHeight = parentWidget()->height();
         const int expandedY =
            std::max(0, parentHeight - kExpandedHeight - kBottomInset);
         targetY = expandedY + ((kExpandedHeight * 3) / 4);
         floorY  = parentHeight;
      }

      const int fillHeight =
         std::max(2, floorY - targetY - lineTop - frameExtra);
      collapsedPeek_->setFixedHeight(fillHeight);

      setFixedHeight(lineTop + fillHeight + frameExtra);

      if (parentWidget() != nullptr)
      {
         const int x = 16;
         move(x, targetY);
      }
   }
   else
   {
      collapsedPeek_->setFixedHeight(2);
      setFixedHeight(264);
   }

   setProperty("collapsed", collapsed_);
   style()->unpolish(this);
   style()->polish(this);

   if (!collapsed_)
   {
      MoveToBottomLeft();
   }
   ClampToParent();
}

void RadarToolboxWidget::MoveToBottomLeft()
{
   QWidget* parent = parentWidget();

   if (parent == nullptr)
   {
      return;
   }

   const int x = 16;
   const int y = std::max(0, parent->height() - height() - 16);

   move(x, y);
}

void RadarToolboxWidget::ClampToParent()
{
   QWidget* parent = parentWidget();

   if (parent == nullptr)
   {
      return;
   }

   const QRect bounds = parent->rect();
   int         x      = this->x();
   int         y      = this->y();

   x = std::max(0, std::min(x, bounds.width() - width()));
   y = std::max(0, std::min(y, bounds.height() - height()));

   move(x, y);
}

} // namespace scwx::qt::ui
