#pragma once

#include <QEvent>
#include <QWidget>

class QVBoxLayout;
class QGridLayout;
class QHBoxLayout;
class QScrollArea;
class QLabel;
class QPushButton;
class QShowEvent;

namespace scwx::qt::ui
{

class RadarToolboxWidget : public QWidget
{
   Q_OBJECT

public:
   explicit RadarToolboxWidget(QWidget* parent = nullptr);

protected:
   bool eventFilter(QObject* watched, QEvent* event) override;
   void showEvent(QShowEvent* event) override;

private:
   void SetCollapsed(bool collapsed);
   void ClampToParent();

   // Root layout
   QVBoxLayout* rootLayout_;

   // Header
   QWidget*     headerContainer_;
   QLabel*      titleLabel_;
   QPushButton* collapseButton_;

   // Products Layout
   QGridLayout* productsLayout_;

   // Playback row
   QWidget*     playbackRow_;
   QPushButton* backBtn_;
   QPushButton* playPauseBtn_;
   QPushButton* forwardBtn_;
   QPushButton* stopBtn_;

   // Scrollable content
   QWidget*     bodyContainer_;
   QScrollArea* scrollArea_;
   QWidget*     scrollContents_;
   QWidget*     headerDivider_;

   bool collapsed_  = false;
   bool isPlaying_  = false;
   bool positioned_ = false;
};

} // namespace scwx::qt::ui
