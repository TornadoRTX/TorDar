#pragma once

#include <scwx/qt/map/generic_layer.hpp>

namespace scwx::qt::map
{

class ColorTableLayer : public GenericLayer
{
   Q_DISABLE_COPY_MOVE(ColorTableLayer)

public:
   explicit ColorTableLayer(std::shared_ptr<gl::GlContext> glContext);
   ~ColorTableLayer();

   void Initialize(const std::shared_ptr<MapContext>& mapContext) final;
   void Render(const std::shared_ptr<MapContext>& mapContext,
               const QMapLibre::CustomLayerRenderParameters&) final;
   void Deinitialize() final;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::map
