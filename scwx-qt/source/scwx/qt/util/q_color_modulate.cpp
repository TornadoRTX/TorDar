#include <scwx/qt/util/q_color_modulate.hpp>

#include <QColor>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QSize>

namespace scwx
{
namespace qt
{
namespace util
{

void modulateColors_(QImage& image, const QColor& color)
{
   for (int y = 0; y < image.height(); ++y)
   {
      QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
      for (int x = 0; x < image.width(); ++x)
      {
         QRgb& rgb   = line[x];
         int   red   = qRed(rgb) * color.redF();
         int   green = qGreen(rgb) * color.greenF();
         int   blue  = qBlue(rgb) * color.blueF();
         int   alpha = qAlpha(rgb) * color.alphaF();

         rgb = qRgba(red, green, blue, alpha);
      }
   }
}

QImage modulateColors(const QImage& image, const QColor& color)
{
   QImage copy = image.copy();
   modulateColors_(copy, color);
   return copy;
}

QPixmap modulateColors(const QPixmap& pixmap, const QColor& color)
{
   QImage image = pixmap.toImage();
   modulateColors_(image, color);
   return QPixmap::fromImage(image);
}

QIcon modulateColors(const QIcon& icon, const QSize& size, const QColor& color)
{
   QPixmap pixmap = modulateColors(icon.pixmap(size), color);
   return QIcon(pixmap);
}

} // namespace util
} // namespace qt
} // namespace scwx
