#pragma once

#include <scwx/qt/types/text_event_key.hpp>

#include <QWidget>

namespace Ui
{
class WarningBoxWidget;
}

namespace scwx
{
namespace qt
{
namespace ui
{

class WarningBoxWidgetImpl;

class WarningBoxWidget : public QWidget
{
   Q_OBJECT

public:
   explicit WarningBoxWidget(QWidget* parent = nullptr);
   ~WarningBoxWidget();

   void ShowWarning(const types::TextEventKey& key);
   void HideWarning();

private slots:
   void on_closeButton_clicked();
   void on_viewEasTextButton_clicked();

private:
   friend class WarningBoxWidgetImpl;
   std::unique_ptr<WarningBoxWidgetImpl> p;
   Ui::WarningBoxWidget*                 ui;
};

} // namespace ui
} // namespace qt
} // namespace scwx
