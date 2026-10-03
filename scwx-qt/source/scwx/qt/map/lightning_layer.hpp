#pragma once

#include <memory>

#include <QObject>

#include <scwx/qt/map/draw_layer.hpp>

namespace scwx::qt::map
{

class LightningLayer : public DrawLayer
{
   Q_OBJECT
   Q_DISABLE_COPY_MOVE(LightningLayer)

public:
   explicit LightningLayer(const std::shared_ptr<gl::GlContext>& glContext);
   ~LightningLayer() override;

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
