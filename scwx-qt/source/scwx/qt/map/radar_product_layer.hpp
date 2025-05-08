#pragma once

#include <scwx/qt/map/generic_layer.hpp>

namespace scwx::qt::map
{

class RadarProductLayerImpl;

class RadarProductLayer : public GenericLayer
{
public:
   explicit RadarProductLayer(const std::shared_ptr<MapContext>& context);
   ~RadarProductLayer();

   void Initialize() override final;
   void Render(const QMapLibre::CustomLayerRenderParameters&) override final;
   void Deinitialize() override final;

   virtual bool
   RunMousePicking(const QMapLibre::CustomLayerRenderParameters& params,
                   const QPointF&                                mouseLocalPos,
                   const QPointF&                                mouseGlobalPos,
                   const glm::vec2&                              mouseCoords,
                   const common::Coordinate&                     mouseGeoCoords,
                   std::shared_ptr<types::EventHandler>& eventHandler) override;

private:
   void UpdateColorTable();
   void UpdateSweep();

private:
   std::unique_ptr<RadarProductLayerImpl> p;
};

} // namespace scwx::qt::map
