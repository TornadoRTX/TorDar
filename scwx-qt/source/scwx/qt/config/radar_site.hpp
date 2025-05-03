#pragma once

#include <scwx/util/time.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <units/length.h>

namespace scwx::qt::config
{

class RadarSiteImpl;

class RadarSite
{
public:
   explicit RadarSite();
   ~RadarSite();

   RadarSite(const RadarSite&)            = delete;
   RadarSite& operator=(const RadarSite&) = delete;

   RadarSite(RadarSite&&) noexcept;
   RadarSite& operator=(RadarSite&&) noexcept;

   [[nodiscard]] std::string                 type() const;
   [[nodiscard]] std::string                 type_name() const;
   [[nodiscard]] std::string                 id() const;
   [[nodiscard]] double                      latitude() const;
   [[nodiscard]] double                      longitude() const;
   [[nodiscard]] std::string                 country() const;
   [[nodiscard]] std::string                 state() const;
   [[nodiscard]] std::string                 place() const;
   [[nodiscard]] std::string                 location_name() const;
   [[nodiscard]] std::string                 tz_name() const;
   [[nodiscard]] units::length::feet<double> altitude() const;

   [[nodiscard]] const scwx::util::time_zone* time_zone() const;

   static std::shared_ptr<RadarSite>              Get(const std::string& id);
   static std::vector<std::shared_ptr<RadarSite>> GetAll();

   /**
    * Find the nearest radar site to the supplied location.
    *
    * @param latitude Latitude in degrees
    * @param longitude Longitude in degrees
    * @param type Restrict results to optional radar type
    *
    * @return Nearest radar site
    */
   static std::shared_ptr<RadarSite>
   FindNearest(double                     latitude,
               double                     longitude,
               std::optional<std::string> type = std::nullopt);

   static void   Initialize();
   static size_t ReadConfig(const std::string& path);

private:
   std::unique_ptr<RadarSiteImpl> p;
};

std::string GetRadarIdFromSiteId(const std::string& siteId);

} // namespace scwx::qt::config
