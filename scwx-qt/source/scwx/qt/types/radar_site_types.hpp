#pragma once

#include <cstdint>

namespace scwx::qt::types
{

enum class RadarSiteStatus : std::uint8_t
{
   Up,          // < 5 minutes
   Warning,     // < 30 minutes
   Down,        // >= 30 minutes
   HighLatency, // > 60 seconds
   Unknown
};

} // namespace scwx::qt::types
