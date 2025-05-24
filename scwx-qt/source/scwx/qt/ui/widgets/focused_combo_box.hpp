#include <QComboBox>
#include <QWheelEvent>

class QFocusedComboBox : public QComboBox
{
   Q_OBJECT

public:
   using QComboBox::QComboBox;

protected:
   void wheelEvent(QWheelEvent* event) override
   {
      if (hasFocus())
      {
         QComboBox::wheelEvent(event);
      }
      else
      {
         event->ignore();
      }
   }
};
