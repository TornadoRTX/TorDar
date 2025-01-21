#include "edit_marker_dialog.hpp"
#include "ui_edit_marker_dialog.h"

#include <scwx/qt/manager/marker_manager.hpp>
#include <scwx/qt/types/marker_types.hpp>
#include <scwx/qt/util/color.hpp>
#include <scwx/qt/util/q_color_modulate.hpp>
#include <scwx/util/logger.hpp>

#include <string>

#include <QObject>
#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QColorDialog>
#include <QPushButton>
#include <QFileDialog>

namespace scwx::qt::ui
{

static const std::string logPrefix_ = "scwx::qt::ui::edit_marker_dialog";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static const QString addingTitle_  = QObject::tr("Add Location Marker");
static const QString editingTitle_ = QObject::tr("Edit Location Marker");

class EditMarkerDialog::Impl
{
public:
   explicit Impl(EditMarkerDialog* self) : self_ {self} {}

   void show_color_dialog();
   void show_icon_file_dialog();

   void set_icon_color(const std::string& color);
   void set_adding(bool adding);

   void connect_signals();

   void handle_accepted();
   void handle_rejected();

   EditMarkerDialog* self_;
   QPushButton*      deleteButton_ {nullptr};
   QIcon             get_colored_icon(const types::MarkerIconInfo& marker,
                                      const std::string&           color);

   std::shared_ptr<manager::MarkerManager> markerManager_ =
      manager::MarkerManager::Instance();
   types::MarkerId editId_ {0};
   bool            adding_ {false};
   std::string     setIconOnAdded_ {""};
};

QIcon EditMarkerDialog::Impl::get_colored_icon(
   const types::MarkerIconInfo& marker, const std::string& color)
{
   return util::modulateColors(marker.qIcon,
                               self_->ui->iconComboBox->iconSize(),
                               QColor(QString::fromStdString(color)));
}

void EditMarkerDialog::Impl::set_adding(bool adding)
{
   if (adding == adding_)
   {
      return;
   }

   if (adding)
   {
      self_->setWindowTitle(addingTitle_);
   }
   else
   {
      self_->setWindowTitle(editingTitle_);
   }
   adding_ = adding;
}

EditMarkerDialog::EditMarkerDialog(QWidget* parent) :
    QDialog(parent),
    p {std::make_unique<Impl>(this)},
    ui(new Ui::EditMarkerDialog)
{
   ui->setupUi(this);

   for (auto& markerIcon : p->markerManager_->get_icons())
   {
      ui->iconComboBox->addItem(
         markerIcon.second.qIcon,
         QString::fromStdString(markerIcon.second.shortName),
         QString::fromStdString(markerIcon.second.name));
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
   setup(0.0, 0.0);
}

void EditMarkerDialog::setup(double latitude, double longitude)
{
   // By default use foreground color as marker color, mainly so the icons
   // are vissable in the dropdown menu.
   const QColor color = QWidget::palette().color(QWidget::foregroundRole());
   p->editId_         = p->markerManager_->add_marker(types::MarkerInfo(
      "",
      latitude,
      longitude,
      manager::MarkerManager::getDefaultIconName(),
      boost::gil::rgba8_pixel_t {static_cast<uint8_t>(color.red()),
                                 static_cast<uint8_t>(color.green()),
                                 static_cast<uint8_t>(color.blue()),
                                 static_cast<uint8_t>(color.alpha())}));

   setup(p->editId_, true);
}

void EditMarkerDialog::setup(types::MarkerId id, bool adding)
{
   std::optional<types::MarkerInfo> marker = p->markerManager_->get_marker(id);
   if (!marker)
   {
      return;
   }

   p->editId_ = id;
   p->set_adding(adding);

   const std::string iconColorStr =
      util::color::ToArgbString(marker->iconColor);
   p->set_icon_color(iconColorStr);

   int iconIndex =
      ui->iconComboBox->findData(QString::fromStdString(marker->iconName));
   if (iconIndex < 0 || marker->iconName == "")
   {
      iconIndex = 0;
   }

   ui->nameLineEdit->setText(QString::fromStdString(marker->name));
   ui->iconComboBox->setCurrentIndex(iconIndex);
   ui->latitudeDoubleSpinBox->setValue(marker->latitude);
   ui->longitudeDoubleSpinBox->setValue(marker->longitude);
   ui->iconColorLineEdit->setText(QString::fromStdString(iconColorStr));
}

types::MarkerInfo EditMarkerDialog::get_marker_info() const
{
   const QString                   colorName = ui->iconColorLineEdit->text();
   const boost::gil::rgba8_pixel_t color =
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
   // WA_DeleteOnClose manages memory
   // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
   auto* dialog = new QColorDialog(self_);

   dialog->setAttribute(Qt::WA_DeleteOnClose);
   dialog->setOption(QColorDialog::ColorDialogOption::ShowAlphaChannel);

   const QColor initialColor(self_->ui->iconColorLineEdit->text());
   if (initialColor.isValid())
   {
      dialog->setCurrentColor(initialColor);
   }

   QObject::connect(dialog,
                    &QColorDialog::colorSelected,
                    self_,
                    [this](const QColor& qColor)
                    {
                       const QString colorName =
                          qColor.name(QColor::NameFormat::HexArgb);
                       self_->ui->iconColorLineEdit->setText(colorName);
                       set_icon_color(colorName.toStdString());
                    });
   dialog->open();
}

void EditMarkerDialog::Impl::show_icon_file_dialog()
{
   auto* dialog = new QFileDialog(self_);

   dialog->setFileMode(QFileDialog::ExistingFile);
   dialog->setNameFilters({"Icon (*.png *.svg)", "All Files (*)"});
   dialog->setAttribute(Qt::WA_DeleteOnClose);

   QObject::connect(dialog,
                    &QFileDialog::fileSelected,
                    self_,
                    [this](const QString& file)
                    {
                       const std::string path =
                          QDir::toNativeSeparators(file).toStdString();
                       setIconOnAdded_ = path;
                       markerManager_->add_icon(path);
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
           [this](const QString& text) { set_icon_color(text.toStdString()); });

   connect(self_->ui->iconColorButton,
           &QAbstractButton::clicked,
           self_,
           [this]() { show_color_dialog(); });

   connect(self_->ui->iconFileOpenButton,
           &QPushButton::clicked,
           self_,
           [this]() { show_icon_file_dialog(); });

   connect(markerManager_.get(),
           &manager::MarkerManager::IconAdded,
           self_,
           [this]()
           {
              const std::string color =
                 self_->ui->iconColorLineEdit->text().toStdString();
              set_icon_color(color);

              if (setIconOnAdded_ != "")
              {
                 const int i = self_->ui->iconComboBox->findData(
                    QString::fromStdString(setIconOnAdded_));
                 if (i >= 0)
                 {
                    self_->ui->iconComboBox->setCurrentIndex(i);
                    setIconOnAdded_ = "";
                 }
              }
           });

   connect(self_->ui->buttonBox->button(QDialogButtonBox::Apply),
           &QAbstractButton::clicked,
           self_,
           [this]() { handle_accepted(); });
}

void EditMarkerDialog::Impl::set_icon_color(const std::string& color)
{
   self_->ui->iconColorFrame->setStyleSheet(
      QString::fromStdString(fmt::format("background-color: {}", color)));

   auto* iconComboBox = self_->ui->iconComboBox;

   const QVariant currentIcon = iconComboBox->currentData();

   self_->ui->iconComboBox->clear();
   for (auto& markerIcon : markerManager_->get_icons())
   {
      const int i =
         iconComboBox->findData(QString::fromStdString(markerIcon.second.name));
      const QIcon icon = get_colored_icon(markerIcon.second, color);
      if (i < 0)
      {
         iconComboBox->addItem(
            icon,
            QString::fromStdString(markerIcon.second.shortName),
            QString::fromStdString(markerIcon.second.name));
      }
      else
      {
         self_->ui->iconComboBox->setItemIcon(i, icon);
      }
   }

   const int i = iconComboBox->findData(currentIcon);
   if (i < 0)
   {
      return;
   }

   iconComboBox->setCurrentIndex(i);
}

void EditMarkerDialog::Impl::handle_accepted()
{
   // switch to editing to that canceling after applying does not delete it
   set_adding(false);
   markerManager_->set_marker(editId_, self_->get_marker_info());
}

void EditMarkerDialog::Impl::handle_rejected()
{
   if (adding_)
   {
      markerManager_->remove_marker(editId_);
   }
}

} // namespace scwx::qt::ui
