#pragma once

#include <scwx/common/geographic.hpp>

#include <vector>

#include <GeographicLib/Geodesic.hpp>
#include <units/angle.h>
#include <units/length.h>

namespace scwx
{
namespace qt
{
namespace util
{
namespace GeographicLib
{

/**
 * Get the default geodesic for the WGS84 ellipsoid.
 *
 * @return WGS84 ellipsoid geodesic
 */
const ::GeographicLib::Geodesic& DefaultGeodesic();

/**
 * Determine if an area/ring, oriented in either direction, contains a point. A
 * point lying on the area boundary is considered to be inside the area.
 *
 * @param [in] area A vector of Coordinates representing the area
 * @param [in] point The point to check against the area
 *
 * @return true if point is inside the area
 */
bool AreaContainsPoint(const std::vector<common::Coordinate>& area,
                       const common::Coordinate&              point);

/**
 * Get the angle between two points.
 *
 * @param [in] lat1 latitude of point 1 (degrees)
 * @param [in] lon1 longitude of point 1 (degrees)
 * @param [in] lat2 latitude of point 2 (degrees)
 * @param [in] lon2 longitude of point 2 (degrees)
 *
 * @return angle between point 1 and point 2
 */
units::angle::degrees<double>
GetAngle(double lat1, double lon1, double lat2, double lon2);

/**
 * Get a coordinate from a polar coordinate offset.
 *
 * @param [in] center The center coordinate from which the angle and distance
 * are given
 * @param [in] angle The angle at which the destination coordinate lies
 * @param [in] distance The distance from the center coordinate to the
 * destination coordinate
 *
 * @return offset coordinate
 */
common::Coordinate GetCoordinate(const common::Coordinate&     center,
                                 units::angle::degrees<double> angle,
                                 units::length::meters<double> distance);

/**
 * Get a coordinate from an (i, j) offset.
 *
 * @param [in] center The center coordinate from which i and j are offset
 * @param [in] i The easting offset in meters
 * @param [in] j The northing offset in meters
 *
 * @return offset coordinate
 */
common::Coordinate GetCoordinate(const common::Coordinate& center,
                                 units::meters<double>     i,
                                 units::meters<double>     j);

/**
 * Get the distance between two points.
 *
 * @param [in] lat1 latitude of point 1 (degrees)
 * @param [in] lon1 longitude of point 1 (degrees)
 * @param [in] lat2 latitude of point 2 (degrees)
 * @param [in] lon2 longitude of point 2 (degrees)
 *
 * @return distance between point 1 and point 2
 */
units::length::meters<double>
GetDistance(double lat1, double lon1, double lat2, double lon2);

/**
 * Get the distance from an area to a point. If the area is less than a quarter
 * radius of the Earth away, this is the closest distance between the area and
 * the point. Otherwise it is the distance from the centroid of the area to the
 * point. Finally, if the point is in the area, it is always 0.
 *
 * @param [in] area A vector of Coordinates representing the area
 * @param [in] point The point to check against the area
 *
 * @return true if area is inside the radius of the point
 */
units::length::meters<double>
GetDistanceAreaPoint(const std::vector<common::Coordinate>& area,
                     const common::Coordinate&              point);

/**
 * Determine if an area/ring, oriented in either direction, is within a
 * distance of a point. A point lying on the area boundary is considered to be
 * inside the area, and thus always in range. Any part of the area being inside
 * the radius counts as inside. Uses GetDistanceAreaPoint to get the distance.
 *
 * @param [in] area A vector of Coordinates representing the area
 * @param [in] point The point to check against the area
 * @param [in] distance The max distance in meters
 *
 * @return true if area is inside the radius of the point
 */
bool AreaInRangeOfPoint(const std::vector<common::Coordinate>& area,
                        const common::Coordinate&              point,
                        const units::length::meters<double>    distance);

/**
 * Get the altitude of the radar beam at a given distance, elevation and height
 *
 * @param [in] range The range to the radar site
 * @param [in] elevation The elevation of the radar site
 * @param [in] height The height of the radar site
 *
 * @return The altitude of the radar at that range
 */
units::length::meters<double>
GetRadarBeamAltititude(units::length::meters<double> range,
                       units::angle::degrees<double> elevation,
                       units::length::meters<double> height);

} // namespace GeographicLib
} // namespace util
} // namespace qt
} // namespace scwx
