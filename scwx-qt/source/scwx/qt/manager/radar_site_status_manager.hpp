#pragma once

#include <scwx/types/nws_types.hpp>

#include <memory>

namespace scwx::qt::manager
{

/**
 * @brief Radar Site Status Manager
 */
class RadarSiteStatusManager
{
public:
   explicit RadarSiteStatusManager();
   ~RadarSiteStatusManager();

   RadarSiteStatusManager(const RadarSiteStatusManager&)            = delete;
   RadarSiteStatusManager& operator=(const RadarSiteStatusManager&) = delete;

   RadarSiteStatusManager(RadarSiteStatusManager&&) noexcept;
   RadarSiteStatusManager& operator=(RadarSiteStatusManager&&) noexcept;

   void Start();
   void Stop();

   static std::shared_ptr<RadarSiteStatusManager> Instance();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::manager
