#pragma once

#include <scwx/qt/map/map_context.hpp>
#include <scwx/qt/types/event_types.hpp>
#include <scwx/common/geographic.hpp>

#include <memory>

#include <QObject>
#include <glm/gtc/type_ptr.hpp>
#include <qmaplibre.hpp>

namespace scwx::qt::map
{

class GenericLayerImpl;

class GenericLayer : public QObject
{
   Q_OBJECT

public:
   explicit GenericLayer(const std::shared_ptr<MapContext>& context);
   virtual ~GenericLayer();

   virtual void Initialize()                                          = 0;
   virtual void Render(const QMapLibre::CustomLayerRenderParameters&) = 0;
   virtual void Deinitialize()                                        = 0;

   /**
    * @brief Run mouse picking on the layer.
    *
    * @param [in] params Custom layer render parameters
    * @param [in] mouseLocalPos Mouse cursor widget position
    * @param [in] mouseGlobalPos Mouse cursor screen position
    * @param [in] mouseCoords Mouse cursor location in map screen coordinates
    * @param [in] mouseGeoCoords Mouse cursor location in geographic coordinates
    * @param [out] eventHandler Event handler associated with picked draw item
    *
    * @return true if a draw item was picked, otherwise false
    */
   virtual bool
   RunMousePicking(const QMapLibre::CustomLayerRenderParameters& params,
                   const QPointF&                                mouseLocalPos,
                   const QPointF&                                mouseGlobalPos,
                   const glm::vec2&                              mouseCoords,
                   const common::Coordinate&                     mouseGeoCoords,
                   std::shared_ptr<types::EventHandler>&         eventHandler);

signals:
   void NeedsRendering();

protected:
   std::shared_ptr<MapContext> context() const;

private:
   std::unique_ptr<GenericLayerImpl> p;
};

} // namespace scwx::qt::map
