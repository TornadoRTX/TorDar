#pragma once

#include <QColor>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QSize>

namespace scwx::qt::util
{

QImage  modulateColors(const QImage& image, const QColor& color);
QPixmap modulateColors(const QPixmap& pixmap, const QColor& color);
QIcon modulateColors(const QIcon& icon, const QSize& size, const QColor& color);

} // namespace scwx::qt::util
