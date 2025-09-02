#include <scwx/network/cpr.hpp>

#include <chrono>

namespace scwx::network::cpr
{

using namespace std::chrono_literals;

static constexpr std::chrono::seconds kConnectTimeout_ = 10s;
static constexpr std::chrono::seconds kTimeout_        = 30s;
static constexpr std::int32_t         kLowSpeedLimit_  = 512; // bytes/sec
static constexpr std::chrono::seconds kLowSpeedTime_   = 15s;

static ::cpr::Header header_ {};

::cpr::ConnectTimeout GetDefaultConnectTimeout()
{
   return ::cpr::ConnectTimeout {kConnectTimeout_};
}

::cpr::Timeout GetDefaultTimeout()
{
   return ::cpr::Timeout {kTimeout_};
}

::cpr::LowSpeed GetDefaultLowSpeed()
{
   return ::cpr::LowSpeed {kLowSpeedLimit_, kLowSpeedTime_};
}

::cpr::Header GetHeader()
{
   return header_;
}

void SetUserAgent(const std::string& userAgent)
{
   header_.insert_or_assign("User-Agent", userAgent);
}

} // namespace scwx::network::cpr
