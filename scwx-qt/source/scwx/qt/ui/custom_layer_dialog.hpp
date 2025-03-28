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
   Q_DISABLE_COPY_MOVE(CustomLayerDialog)

public:
   explicit CustomLayerDialog(const QMapLibre::Settings& settings,
                              QWidget*                   parent = nullptr);
   ~CustomLayerDialog() override;

   std::string selected_layer();

private:
   friend class CustomLayerDialogImpl;
   std::unique_ptr<CustomLayerDialogImpl> p;
   Ui::CustomLayerDialog*                 ui;
};

} // namespace scwx::qt::ui
