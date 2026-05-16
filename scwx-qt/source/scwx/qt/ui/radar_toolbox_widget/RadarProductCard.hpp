#pragma once

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QPoint>

class QLabel;

namespace scwx::qt::ui
{

class RadarProductCard : public QWidget
{
   Q_OBJECT

public:
   explicit RadarProductCard(const QString& title,
                             const QString& iconPath,
                             QWidget*       parent = nullptr);
   QSize sizeHint() const override;
   void  SetSelected(bool selected);

signals:
   void Clicked();
   void RightClicked(const QPoint& globalPos);

protected:
   void mousePressEvent(QMouseEvent* event) override;
   void enterEvent(QEnterEvent* event) override;
   void leaveEvent(QEvent* event) override;

private:
   void UpdateStyle();

   QLabel*  imageLabel_;
   QLabel*  titleLabel_;
   QWidget* iconContainer_;

   bool hovered_  = false;
   bool selected_ = false;
};

} // namespace scwx::qt::ui
