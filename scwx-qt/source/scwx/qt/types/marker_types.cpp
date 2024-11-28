#include <scwx/qt/types/marker_types.hpp>

namespace scwx
{
namespace qt
{
namespace types
{

const std::vector<MarkerIconInfo>& getMarkerIcons()
{
   static std::vector<MarkerIconInfo> markerIcons = {};
   if (markerIcons.size() == 0)
   {
      markerIcons = {
         MarkerIconInfo(types::ImageTexture::LocationMarker, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationPin, 6, 16),
         MarkerIconInfo(types::ImageTexture::LocationCrosshair, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationStar, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationBriefcase, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationBuildingColumns, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationBuilding, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationCaravan, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationHouse, -1, -1),
         MarkerIconInfo(types::ImageTexture::LocationTent, -1, -1),
      };
   }

   return markerIcons;
}

} // namespace types
} // namespace qt
} // namespace scwx
