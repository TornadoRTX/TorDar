#pragma once

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QEnterEvent>

class QLabel;

namespace scwx::qt::ui
{

class RadarProductCard : public QWidget
{
   Q_OBJECT

public:
   explicit RadarProductCard(const QString& title,
                             const QString& subtitle = QString(),
                             QWidget*       parent   = nullptr);

signals:
   void Clicked();

protected:
   void mousePressEvent(QMouseEvent* event) override;
   void enterEvent(QEnterEvent* event) override;
   void leaveEvent(QEvent* event) override;

private:
   void UpdateStyle();

   QLabel* titleLabel_;
   QLabel* subtitleLabel_;

   bool hovered_ = false;
};

} // namespace scwx::qt::ui