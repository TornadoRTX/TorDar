#pragma once

#include <scwx/qt/map/draw_layer.hpp>

#include <string>

namespace scwx::qt::map
{

class PlacefileLayer : public DrawLayer
{
   Q_OBJECT
   Q_DISABLE_COPY_MOVE(PlacefileLayer)

public:
   explicit PlacefileLayer(const std::shared_ptr<gl::GlContext>& glContext,
                           const std::string&                    placefileName);
   ~PlacefileLayer();

   std::string placefile_name() const;

   void set_placefile_name(const std::string& placefileName);

   void Initialize(const std::shared_ptr<MapContext>& mapContext) final;
   void Render(const std::shared_ptr<MapContext>& mapContext,
               const QMapLibre::CustomLayerRenderParameters&) final;
   void Deinitialize() final;

   void ReloadData();

signals:
   void DataReloaded();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::map
