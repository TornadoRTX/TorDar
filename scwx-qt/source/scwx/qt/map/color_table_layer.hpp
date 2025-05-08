#pragma once

#include <scwx/qt/map/generic_layer.hpp>

namespace scwx::qt::map
{

class ColorTableLayerImpl;

class ColorTableLayer : public GenericLayer
{
public:
   explicit ColorTableLayer(const std::shared_ptr<MapContext>& context);
   ~ColorTableLayer();

   void Initialize() override final;
   void Render(const QMapLibre::CustomLayerRenderParameters&) override final;
   void Deinitialize() override final;

private:
   std::unique_ptr<ColorTableLayerImpl> p;
};

} // namespace scwx::qt::map
