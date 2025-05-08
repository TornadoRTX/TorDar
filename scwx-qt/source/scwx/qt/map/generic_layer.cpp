#include <scwx/qt/map/generic_layer.hpp>

namespace scwx::qt::map
{

class GenericLayerImpl
{
public:
   explicit GenericLayerImpl(std::shared_ptr<MapContext> context) :
       context_ {std::move(context)}
   {
   }

   ~GenericLayerImpl() {}

   std::shared_ptr<MapContext> context_;
};

GenericLayer::GenericLayer(const std::shared_ptr<MapContext>& context) :
    p(std::make_unique<GenericLayerImpl>(context))
{
}
GenericLayer::~GenericLayer() = default;

bool GenericLayer::RunMousePicking(
   const QMapLibre::CustomLayerRenderParameters& /* params */,
   const QPointF& /* mouseLocalPos */,
   const QPointF& /* mouseGlobalPos */,
   const glm::vec2& /* mousePos */,
   const common::Coordinate& /* mouseGeoCoords */,
   std::shared_ptr<types::EventHandler>& /* eventHandler */)
{
   // By default, the layer has nothing to pick
   return false;
}

std::shared_ptr<MapContext> GenericLayer::context() const
{
   return p->context_;
}

} // namespace scwx::qt::map
