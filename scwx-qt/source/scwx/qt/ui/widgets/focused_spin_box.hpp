#pragma once

#include <QSpinBox>
#include <QWheelEvent>

class QFocusedSpinBox : public QSpinBox
{
   Q_OBJECT

public:
   using QSpinBox::QSpinBox;

protected:
   void wheelEvent(QWheelEvent* event) override
   {
      if (hasFocus())
      {
         QSpinBox::wheelEvent(event);
      }
      else
      {
         event->ignore();
      }
   }
};
