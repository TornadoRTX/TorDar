#pragma once

#include <QDialog>

namespace Ui
{
class HighPrivilegeDialog;
}

namespace scwx
{
namespace qt
{
namespace ui
{

class HighPrivilegeDialogImpl;

class HighPrivilegeDialog : public QDialog
{
   Q_OBJECT

private:
   Q_DISABLE_COPY(HighPrivilegeDialog)

public:
   explicit HighPrivilegeDialog(QWidget* parent = nullptr);
   ~HighPrivilegeDialog();

   bool disable_high_privilege_message();

private:
   friend HighPrivilegeDialogImpl;
   std::unique_ptr<HighPrivilegeDialogImpl> p;
   Ui::HighPrivilegeDialog*                 ui;
};

} // namespace ui
} // namespace qt
} // namespace scwx
