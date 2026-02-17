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
   void SetLevel2ProductsWidget(QWidget* widget);
   void SetLevel3ProductsWidget(QWidget* widget);

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

   // Scrollable content
   QScrollArea* scrollArea_;
   QWidget*     scrollContents_;

   bool collapsed_ = false;
   bool isPlaying_ = false;
};

} // namespace scwx::qt::ui