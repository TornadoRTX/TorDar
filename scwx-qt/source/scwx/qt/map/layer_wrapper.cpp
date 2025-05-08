#include <scwx/qt/map/layer_wrapper.hpp>

namespace scwx::qt::map
{

class LayerWrapperImpl
{
public:
   explicit LayerWrapperImpl(std::shared_ptr<GenericLayer> layer) :
       layer_ {std::move(layer)}
   {
   }

   ~LayerWrapperImpl() {}

   std::shared_ptr<GenericLayer> layer_;
};

LayerWrapper::LayerWrapper(const std::shared_ptr<GenericLayer>& layer) :
    p(std::make_unique<LayerWrapperImpl>(layer))
{
}
LayerWrapper::~LayerWrapper() = default;

LayerWrapper::LayerWrapper(LayerWrapper&&) noexcept            = default;
LayerWrapper& LayerWrapper::operator=(LayerWrapper&&) noexcept = default;

void LayerWrapper::initialize()
{
   auto& layer = p->layer_;
   if (layer != nullptr)
   {
      layer->Initialize();
   }
}

void LayerWrapper::render(const QMapLibre::CustomLayerRenderParameters& params)
{
   auto& layer = p->layer_;
   if (layer != nullptr)
   {
      layer->Render(params);
   }
}

void LayerWrapper::deinitialize()
{
   // Ensure layers are not retained after call to deinitialize
   auto& layer = p->layer_;
   if (layer != nullptr)
   {
      layer->Deinitialize();
      layer = nullptr;
   }
}

} // namespace scwx::qt::map
