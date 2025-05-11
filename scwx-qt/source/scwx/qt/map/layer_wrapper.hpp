#pragma once

#include <scwx/qt/map/generic_layer.hpp>
#include <scwx/qt/map/map_context.hpp>

namespace scwx::qt::map
{

class LayerWrapper : public QMapLibre::CustomLayerHostInterface
{
public:
   explicit LayerWrapper(std::shared_ptr<GenericLayer> layer,
                         std::shared_ptr<MapContext>   mapContext);
   ~LayerWrapper();

   LayerWrapper(const LayerWrapper&)            = delete;
   LayerWrapper& operator=(const LayerWrapper&) = delete;

   LayerWrapper(LayerWrapper&&) noexcept;
   LayerWrapper& operator=(LayerWrapper&&) noexcept;

   void initialize() final;
   void render(const QMapLibre::CustomLayerRenderParameters&) final;
   void deinitialize() final;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::map
