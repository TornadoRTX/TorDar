#pragma once

#include <qmaplibre.hpp>

namespace scwx::qt::map::RadarRangeLayer
{

void Add(std::shared_ptr<QMapLibre::Map> map,
         float                           range,
         QMapLibre::Coordinate           center,
         const QString&                  before = QString());
void Update(std::shared_ptr<QMapLibre::Map> map,
            float                           range,
            QMapLibre::Coordinate           center);

} // namespace scwx::qt::map::RadarRangeLayer
