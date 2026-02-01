#pragma once

#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QLabel;
class QPushButton;

namespace scwx::qt::ui
{

class RadarToolboxWidget : public QWidget
{
   Q_OBJECT

public:
   explicit RadarToolboxWidget(QWidget* parent = nullptr);

private:
   // Root layout
   QVBoxLayout* rootLayout_;

   // Header
   QWidget*     headerContainer_;
   QLabel*      titleLabel_;
   QPushButton* collapseButton_;

   // Playback row
   QWidget* playbackRow_;

   // Scrollable content
   QScrollArea* scrollArea_;
   QWidget*     scrollContents_;

   bool collapsed_ = false;
};

} // namespace scwx::qt::ui
