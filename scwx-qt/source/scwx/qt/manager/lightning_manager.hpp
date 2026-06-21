#pragma once

#include <memory>
#include <optional>
#include <string>

#include <QObject>

#include <scwx/qt/config/radar_site.hpp>
#include <scwx/gr/placefile.hpp>

namespace scwx::qt::manager
{

class LightningManager : public QObject
{
   Q_OBJECT

public:
   explicit LightningManager();
   ~LightningManager() override;

   static std::shared_ptr<LightningManager> Instance();

   void SetRadarSite(std::shared_ptr<config::RadarSite> radarSite);
   void SetRadarScanRange(std::optional<float> radarRangeKm);
   void NotifyMapZoom(double zoom);

   [[nodiscard]] std::shared_ptr<gr::Placefile> placefile() const;

   [[nodiscard]] static bool IsLightningPlacefile(const std::string& name);

signals:
   void LightningUpdated();
   void ViewUpdated();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::manager
