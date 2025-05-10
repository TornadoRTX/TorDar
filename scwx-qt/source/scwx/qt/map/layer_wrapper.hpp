#pragma once

#include <scwx/qt/map/generic_layer.hpp>

namespace scwx::qt::map
{

class LayerWrapperImpl;

class LayerWrapper : public QMapLibre::CustomLayerHostInterface
{
public:
   explicit LayerWrapper(const std::shared_ptr<GenericLayer>& layer);
   ~LayerWrapper();

   LayerWrapper(const LayerWrapper&)            = delete;
   LayerWrapper& operator=(const LayerWrapper&) = delete;

   LayerWrapper(LayerWrapper&&) noexcept;
   LayerWrapper& operator=(LayerWrapper&&) noexcept;

   void initialize() override final;
   void render(const QMapLibre::CustomLayerRenderParameters&) override final;
   void deinitialize() override final;

private:
   std::unique_ptr<LayerWrapperImpl> p;
};

} // namespace scwx::qt::map
