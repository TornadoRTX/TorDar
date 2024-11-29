#include "edit_marker_dialog.hpp"
#include "ui_edit_marker_dialog.h"

#include <scwx/qt/manager/marker_manager.hpp>
#include <scwx/qt/types/marker_types.hpp>
#include <scwx/qt/util/color.hpp>
#include <scwx/qt/util/q_color_modulate.hpp>
#include <scwx/util/logger.hpp>

#include <string>
#include <vector>

#include <QObject>
#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QColorDialog>
#include <QPushButton>

namespace scwx
{
namespace qt
{
namespace ui
{

static const std::string logPrefix_ = "scwx::qt::ui::edit_marker_dialog";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class EditMarkerDialog::Impl
{
public:
   explicit Impl(EditMarkerDialog* self) :
      self_{self}
   {
   }

   void show_color_dialog();
   void set_icon_color(const std::string& color);

   void connect_signals();

   void handle_accepted();
   void handle_rejected();

   EditMarkerDialog* self_;
   QPushButton* deleteButton_;
   QIcon get_colored_icon(size_t index, const std::string& color);

   std::shared_ptr<manager::MarkerManager> markerManager_ =
      manager::MarkerManager::Instance();
   const std::vector<types::MarkerIconInfo>* icons_;
   types::MarkerId editId_;
   bool adding_;
};

QIcon EditMarkerDialog::Impl::get_colored_icon(size_t             index,
                                               const std::string& color)
{
   if (index >= icons_->size())
   {
      return QIcon();
   }

   return util::modulateColors((*icons_)[index].qIcon,
                               self_->ui->iconComboBox->iconSize(),
                               QColor(QString::fromStdString(color)));
}

EditMarkerDialog::EditMarkerDialog(QWidget* parent) :
   QDialog(parent),
   p {std::make_unique<Impl>(this)},
   ui(new Ui::EditMarkerDialog)
{
   ui->setupUi(this);

   p->icons_ = &types::getMarkerIcons();
   for (auto& markerIcon : (*p->icons_))
   {
      ui->iconComboBox->addItem(markerIcon.qIcon,
                                QString(""),
                                QString::fromStdString(markerIcon.name));
   }
   p->deleteButton_ =
      ui->buttonBox->addButton("Delete", QDialogButtonBox::DestructiveRole);
   p->connect_signals();
}

EditMarkerDialog::~EditMarkerDialog()
{
   delete ui;
}

void EditMarkerDialog::setup()
{
   setup(0, 0);
}

void EditMarkerDialog::setup(double latitude, double longitude)
{
   ui->iconComboBox->setCurrentIndex(0);
   // By default use foreground color as marker color, mainly so the icons
   // are vissable in the dropdown menu.
   QColor color = QWidget::palette().color(QWidget::foregroundRole());
   p->editId_ = p->markerManager_->add_marker(types::MarkerInfo(
      "",
      latitude,
      longitude,
      ui->iconComboBox->currentData().toString().toStdString(),
      boost::gil::rgba8_pixel_t {static_cast<uint8_t>(color.red()),
                                 static_cast<uint8_t>(color.green()),
                                 static_cast<uint8_t>(color.blue()),
                                 static_cast<uint8_t>(color.alpha())}));

   setup(p->editId_);
   p->adding_ = true;
}

void EditMarkerDialog::setup(types::MarkerId id)
{
   std::optional<types::MarkerInfo> marker =
      p->markerManager_->get_marker(id);
   if (!marker)
   {
      return;
   }

   p->editId_ = id;
   p->adding_ = false;

   int iconIndex =
      ui->iconComboBox->findData(QString::fromStdString(marker->iconName));
   if (iconIndex < 0 || marker->iconName == "")
   {
      iconIndex = 0;
   }

   std::string iconColorStr = util::color::ToArgbString(marker->iconColor);

   ui->nameLineEdit->setText(QString::fromStdString(marker->name));
   ui->iconComboBox->setCurrentIndex(iconIndex);
   ui->latitudeDoubleSpinBox->setValue(marker->latitude);
   ui->longitudeDoubleSpinBox->setValue(marker->longitude);
   ui->iconColorLineEdit->setText(QString::fromStdString(iconColorStr));

   p->set_icon_color(iconColorStr);
}

types::MarkerInfo EditMarkerDialog::get_marker_info() const
{
   QString colorName = ui->iconColorLineEdit->text();
   boost::gil::rgba8_pixel_t color =
      util::color::ToRgba8PixelT(colorName.toStdString());

   return types::MarkerInfo(
      ui->nameLineEdit->text().toStdString(),
      ui->latitudeDoubleSpinBox->value(),
      ui->longitudeDoubleSpinBox->value(),
      ui->iconComboBox->currentData().toString().toStdString(),
      color);
}

void EditMarkerDialog::Impl::show_color_dialog()
{

   QColorDialog* dialog = new QColorDialog(self_);

   dialog->setAttribute(Qt::WA_DeleteOnClose);
   dialog->setOption(QColorDialog::ColorDialogOption::ShowAlphaChannel);

   QColor initialColor(self_->ui->iconColorLineEdit->text());
   if (initialColor.isValid())
   {
      dialog->setCurrentColor(initialColor);
   }

   QObject::connect(dialog,
                    &QColorDialog::colorSelected,
                    self_,
                    [this](const QColor& qColor)
                    {
                       QString colorName =
                          qColor.name(QColor::NameFormat::HexArgb);
                       self_->ui->iconColorLineEdit->setText(colorName);
                       set_icon_color(colorName.toStdString());
                    });
   dialog->open();
}

void EditMarkerDialog::Impl::connect_signals()
{
   connect(self_,
           &EditMarkerDialog::accepted,
           self_,
           [this]() { handle_accepted(); });

   connect(self_,
           &EditMarkerDialog::rejected,
           self_,
           [this]() { handle_rejected(); });

   connect(deleteButton_,
           &QPushButton::clicked,
           self_,
           [this]()
           {
              markerManager_->remove_marker(editId_);
              self_->done(0);
           });

   connect(self_->ui->iconColorLineEdit,
           &QLineEdit::textEdited,
           self_,
           [=, this](const QString& text)
           { set_icon_color(text.toStdString()); });

   connect(self_->ui->iconColorButton,
           &QAbstractButton::clicked,
           self_,
           [=, this]() { self_->p->show_color_dialog(); });
}

void EditMarkerDialog::Impl::set_icon_color(const std::string& color)
{
   self_->ui->iconColorFrame->setStyleSheet(
      QString::fromStdString(fmt::format("background-color: {}", color)));

   for (size_t i = 0; i < icons_->size(); i++)
   {
      self_->ui->iconComboBox->setItemIcon(static_cast<int>(i),
                                           get_colored_icon(i, color));
   }
}

void EditMarkerDialog::Impl::handle_accepted()
{
   markerManager_->set_marker(editId_, self_->get_marker_info());
}

void EditMarkerDialog::Impl::handle_rejected()
{
   if (adding_)
   {
      markerManager_->remove_marker(editId_);
   }
}

} // namespace ui
} // namespace qt
} // namespace scwx
