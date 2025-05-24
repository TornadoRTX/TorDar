#include <QDoubleSpinBox>
#include <QWheelEvent>

class QFocusedDoubleSpinBox : public QDoubleSpinBox
{
   Q_OBJECT

public:
   using QDoubleSpinBox::QDoubleSpinBox;

protected:
   void wheelEvent(QWheelEvent* event) override
   {
      if (hasFocus())
      {
         QDoubleSpinBox::wheelEvent(event);
      }
      else
      {
         event->ignore();
      }
   }
};
