#include <scwx/qt/util/q_color_modulate.hpp>

#include <QColor>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QSize>

namespace scwx::qt::util
{

void modulateColors_(QImage& image, const QColor& color)
{
   for (int y = 0; y < image.height(); ++y)
   {
      QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
      for (int x = 0; x < image.width(); ++x)
      {
         // This is pulled from Qt Documentation
         // https://doc.qt.io/qt-6/qimage.html#scanLine
         // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
         QRgb& rgb = line[x];
         /* clang-format off
          * NOLINTBEGIN(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
          * qRed/qGreen/qBlue/qAlpha return values 0-255, handlable by float
          * redF/greenF/blueF/alphaF are all 0-1, so output is 0-255
          * Rounding is fine for this.
          * clang-format on
          */
         const int red   = qRed(rgb) * color.redF();
         const int green = qGreen(rgb) * color.greenF();
         const int blue  = qBlue(rgb) * color.blueF();
         const int alpha = qAlpha(rgb) * color.alphaF();
         /* clang-format off
          * NOLINTEND(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
          * clang-format on
          */

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
   const QPixmap pixmap = modulateColors(icon.pixmap(size), color);
   return QIcon(pixmap);
}

} // namespace scwx::qt::util
