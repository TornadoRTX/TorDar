#pragma once

#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QLabel;
class QPushButton;

namespace scwx::qt::ui
{

class AnimationDockWidget;
class CollapsibleGroup;

class RadarToolboxWidget : public QWidget
{
   Q_OBJECT

public:
   explicit RadarToolboxWidget(QWidget* parent = nullptr);
   AnimationDockWidget* animation_dock_widget() const;
   CollapsibleGroup*    timeline_group() const;

private:
   // Root layout
   QVBoxLayout* rootLayout_;

   // Header
   QWidget*     headerContainer_;
   QLabel*      titleLabel_;
   QPushButton* collapseButton_;

   // Products Layout
   QVBoxLayout* productsLayout_;

   // Playback row
   QWidget*     playbackRow_;
   QPushButton* backBtn_;
   QPushButton* playPauseBtn_;
   QPushButton* forwardBtn_;
   QPushButton* stopBtn_;

   // Timeline
   CollapsibleGroup*    timelineGroup_;
   AnimationDockWidget* animationDockWidget_;

   // Scrollable content
   QScrollArea* scrollArea_;
   QWidget*     scrollContents_;

   bool collapsed_ = false;
   bool isPlaying_ = false;
};

} // namespace scwx::qt::ui
