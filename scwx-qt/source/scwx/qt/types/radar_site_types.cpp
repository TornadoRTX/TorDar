#include <scwx/qt/types/radar_site_types.hpp>
#include <scwx/util/enum.hpp>

namespace scwx::qt::types
{

static const std::unordered_map<RadarSiteStatus, std::string>
   radarSiteStatusName_ {{RadarSiteStatus::Up, "Up"},
                         {RadarSiteStatus::Warning, "Warning"},
                         {RadarSiteStatus::Down, "Down"},
                         {RadarSiteStatus::HighLatency, "HighLatency"},
                         {RadarSiteStatus::Unknown, "?"}};

SCWX_GET_ENUM(RadarSiteStatus, GetRadarSiteStatus, radarSiteStatusName_)

const std::string& GetRadarSiteStatusName(RadarSiteStatus status)
{
   return radarSiteStatusName_.at(status);
}

} // namespace scwx::qt::types
