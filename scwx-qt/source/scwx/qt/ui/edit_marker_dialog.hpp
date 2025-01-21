#pragma once
#include <scwx/qt/types/marker_types.hpp>

#include <QDialog>

namespace Ui
{
class EditMarkerDialog;
}

namespace scwx::qt::ui
{
class EditMarkerDialog : public QDialog
{
   Q_OBJECT
   Q_DISABLE_COPY_MOVE(EditMarkerDialog)

public:
   explicit EditMarkerDialog(QWidget* parent = nullptr);
   ~EditMarkerDialog() override;

   void setup();
   void setup(double latitude, double longitude);
   void setup(types::MarkerId id, bool adding = false);

   [[nodiscard]] types::MarkerInfo get_marker_info() const;

private:
   class Impl;
   std::unique_ptr<Impl> p;
   Ui::EditMarkerDialog* ui;
};

} // namespace scwx::qt::ui
