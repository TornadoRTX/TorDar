#pragma once

#include <qmaplibre.hpp>
#include <QDialog>
#include <string>

namespace Ui
{
class CustomLayerDialog;
}

namespace scwx::qt::ui
{

class CustomLayerDialogImpl;

class CustomLayerDialog : public QDialog
{
   Q_OBJECT

public:
   explicit CustomLayerDialog(const QMapLibre::Settings& settings,
                              QWidget*                   parent = nullptr);
   ~CustomLayerDialog() override;
   CustomLayerDialog(const CustomLayerDialog&)            = delete;
   CustomLayerDialog(CustomLayerDialog&&)                 = delete;
   CustomLayerDialog& operator=(const CustomLayerDialog&) = delete;
   CustomLayerDialog& operator=(CustomLayerDialog&&)      = delete;

   std::string selected_layer();

private:
   friend class CustomLayerDialogImpl;
   std::unique_ptr<CustomLayerDialogImpl> p;
   Ui::CustomLayerDialog*                 ui;
};

} // namespace scwx::qt::ui
