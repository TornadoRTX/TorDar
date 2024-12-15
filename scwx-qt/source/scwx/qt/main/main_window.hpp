#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

namespace scwx
{
namespace qt
{
namespace main
{

class MainWindowImpl;

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(QWidget* parent = nullptr);
   ~MainWindow();

   void keyPressEvent(QKeyEvent* ev) override final;
   void keyReleaseEvent(QKeyEvent* ev) override final;
   void showEvent(QShowEvent* event) override;
   void closeEvent(QCloseEvent* event) override;

signals:
   void ActiveMapMoved(double latitude, double longitude);

private slots:
   void on_actionOpenNexrad_triggered();
   void on_actionOpenTextEvent_triggered();
   void on_actionSettings_triggered();
   void on_actionExit_triggered();
   void on_actionGpsInfo_triggered();
   void on_actionColorTable_triggered(bool checked);
   void on_actionRadarRange_triggered(bool checked);
   void on_actionRadarSites_triggered(bool checked);
   void on_actionPlacefileManager_triggered();
   void on_actionMarkerManager_triggered();
   void on_actionLayerManager_triggered();
   void on_actionImGuiDebug_triggered();
   void on_actionDumpLayerList_triggered();
   void on_actionDumpRadarProductRecords_triggered();
   void on_actionRadarWireframe_triggered(bool checked);
   void on_actionUserManual_triggered();
   void on_actionDiscord_triggered();
   void on_actionGitHubRepository_triggered();
   void on_actionCheckForUpdates_triggered();
   void on_actionAboutSupercellWx_triggered();
   void on_radarSiteHomeButton_clicked();
   void on_radarSiteSelectButton_clicked();

private:
   std::unique_ptr<MainWindowImpl> p;
   Ui::MainWindow*                 ui;

   friend class MainWindowImpl;
};

} // namespace main
} // namespace qt
} // namespace scwx
