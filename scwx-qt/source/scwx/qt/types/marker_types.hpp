#pragma once

#include <scwx/qt/types/texture_types.hpp>

#include <string>
#include <cstdint>

#include <boost/gil.hpp>
#include <QIcon>

namespace scwx
{
namespace qt
{
namespace types
{
typedef std::uint64_t MarkerId;

struct MarkerInfo
{
   MarkerInfo(const std::string&        name,
              double                    latitude,
              double                    longitude,
              const std::string         iconName,
              boost::gil::rgba8_pixel_t iconColor) :
       name {name},
       latitude {latitude},
       longitude {longitude},
       iconName {iconName},
       iconColor {iconColor}
   {
   }

   MarkerId                  id;
   std::string               name;
   double                    latitude;
   double                    longitude;
   std::string               iconName;
   boost::gil::rgba8_pixel_t iconColor;
};

struct MarkerIconInfo {
   explicit MarkerIconInfo(types::ImageTexture texture,
                           std::int32_t        hotX,
                           std::int32_t        hotY) :
      name{types::GetTextureName(texture)},
      path{types::GetTexturePath(texture)},
      hotX{hotX},
      hotY{hotY},
      qIcon{QIcon(QString::fromStdString(path))}
   {
   }

   std::string name;
   std::string path;
   std::int32_t hotX;
   std::int32_t hotY;
   QIcon qIcon;
};

const std::vector<MarkerIconInfo>& getMarkerIcons();

} // namespace types
} // namespace qt
} // namespace scwx
