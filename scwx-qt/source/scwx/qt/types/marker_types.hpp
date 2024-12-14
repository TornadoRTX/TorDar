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
using MarkerId = std::uint64_t;

struct MarkerInfo
{
   MarkerInfo(const std::string&               name,
              double                           latitude,
              double                           longitude,
              const std::string&               iconName,
              const boost::gil::rgba8_pixel_t& iconColor) :
       name {name},
       latitude {latitude},
       longitude {longitude},
       iconName {iconName},
       iconColor {iconColor}
   {
   }

   MarkerId                  id {0};
   std::string               name;
   double                    latitude;
   double                    longitude;
   std::string               iconName;
   boost::gil::rgba8_pixel_t iconColor;
};

struct MarkerIconInfo
{
   explicit MarkerIconInfo(types::ImageTexture texture,
                           std::int32_t        hotX,
                           std::int32_t        hotY) :
       name {types::GetTextureName(texture)},
       path {types::GetTexturePath(texture)},
       hotX {hotX},
       hotY {hotY},
       qIcon {QIcon(QString::fromStdString(path))},
       image {}
   {
   }

   explicit MarkerIconInfo(const std::string&                         path,
                           std::int32_t                               hotX,
                           std::int32_t                               hotY,
                           std::shared_ptr<boost::gil::rgba8_image_t> image) :
       name {path},
       path {path},
       hotX {hotX},
       hotY {hotY},
       qIcon {QIcon(QString::fromStdString(path))},
       image {image}
   {
   }

   std::string                                               name;
   std::string                                               path;
   std::int32_t                                              hotX;
   std::int32_t                                              hotY;
   QIcon                                                     qIcon;
   std::optional<std::shared_ptr<boost::gil::rgba8_image_t>> image;
};

} // namespace types
} // namespace qt
} // namespace scwx
