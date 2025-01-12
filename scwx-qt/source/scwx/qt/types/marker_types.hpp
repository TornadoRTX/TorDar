#pragma once

#include <scwx/qt/types/texture_types.hpp>

#include <cstdint>
#include <string>
#include <utility>

#include <boost/gil.hpp>
#include <QFileInfo>
#include <QIcon>

namespace scwx::qt::types
{
using MarkerId = std::uint64_t;

struct MarkerInfo
{
   MarkerInfo(std::string                      name,
              double                           latitude,
              double                           longitude,
              std::string                      iconName,
              const boost::gil::rgba8_pixel_t& iconColor) :
       name {std::move(name)},
       latitude {latitude},
       longitude {longitude},
       iconName {std::move(iconName)},
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
   // Initializer for default icons (which use a texture)
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
      auto        qName = QString::fromStdString(name);
      QStringList parts = qName.split("location-");
      shortName         = parts.last().toStdString();
   }

   // Initializer for custom icons (which use a file path)
   explicit MarkerIconInfo(const std::string&                         path,
                           std::int32_t                               hotX,
                           std::int32_t                               hotY,
                           std::shared_ptr<boost::gil::rgba8_image_t> image) :
       name {path},
       path {path},
       shortName {QFileInfo(path.c_str()).fileName().toStdString()},
       hotX {hotX},
       hotY {hotY},
       qIcon {QIcon(QString::fromStdString(path))},
       image {image}
   {
   }

   std::string                                               name;
   std::string                                               path;
   std::string                                               shortName;
   std::int32_t                                              hotX;
   std::int32_t                                              hotY;
   QIcon                                                     qIcon;
   std::optional<std::shared_ptr<boost::gil::rgba8_image_t>> image;
};

} // namespace scwx::qt::types
