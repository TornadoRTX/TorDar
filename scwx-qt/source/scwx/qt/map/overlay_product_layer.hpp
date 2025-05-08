#pragma once

#include <scwx/qt/map/draw_layer.hpp>

namespace scwx::qt::map
{

class OverlayProductLayer : public DrawLayer
{
public:
   explicit OverlayProductLayer(const std::shared_ptr<MapContext>& context);
   ~OverlayProductLayer();

   void Initialize() override final;
   void Render(const QMapLibre::CustomLayerRenderParameters&) override final;
   void Deinitialize() override final;

   bool RunMousePicking(
      const QMapLibre::CustomLayerRenderParameters& params,
      const QPointF&                                mouseLocalPos,
      const QPointF&                                mouseGlobalPos,
      const glm::vec2&                              mouseCoords,
      const common::Coordinate&                     mouseGeoCoords,
      std::shared_ptr<types::EventHandler>& eventHandler) override final;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::map
