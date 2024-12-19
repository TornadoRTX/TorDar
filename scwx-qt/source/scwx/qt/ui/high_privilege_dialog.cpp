#include "high_privilege_dialog.hpp"
#include "ui_high_privilege_dialog.h"

#include <scwx/util/logger.hpp>
#include <QPushButton>

namespace scwx
{
namespace qt
{
namespace ui
{

static const std::string logPrefix_ = "scwx::qt::ui::high_privilege_dialog";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class HighPrivilegeDialogImpl
{
public:
   explicit HighPrivilegeDialogImpl(HighPrivilegeDialog* self) :
       self_ {self} {};
   ~HighPrivilegeDialogImpl() = default;

   HighPrivilegeDialog* self_;
};

HighPrivilegeDialog::HighPrivilegeDialog(QWidget* parent) :
    QDialog(parent),
    p {std::make_unique<HighPrivilegeDialogImpl>(this)},
    ui(new Ui::HighPrivilegeDialog)
{
   ui->setupUi(this);
}

bool HighPrivilegeDialog::disable_high_privilege_message()
{
   return ui->highPrivilegeCheckBox->isChecked();
}

HighPrivilegeDialog::~HighPrivilegeDialog()
{
   delete ui;
}

} // namespace ui
} // namespace qt
} // namespace scwx
