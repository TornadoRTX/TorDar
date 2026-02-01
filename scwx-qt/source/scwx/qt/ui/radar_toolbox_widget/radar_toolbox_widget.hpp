#pragma once

#include <QWidget>

class QVBoxLayout;
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
   QVBoxLayout* rootLayout_;
   QWidget*     header_;
   QWidget*     playbackRow_;
   QScrollArea* scrollArea_;
   QWidget*     scrollContents_;
};

} // namespace scwx::qt::ui
