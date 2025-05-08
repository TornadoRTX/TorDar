#pragma once

#include <scwx/qt/gl/draw/draw_item.hpp>
#include <scwx/qt/map/generic_layer.hpp>

namespace scwx::qt::map
{

class DrawLayerImpl;

class DrawLayer : public GenericLayer
{
public:
   explicit DrawLayer(const std::shared_ptr<MapContext>& context,
                      const std::string&                 imGuiContextName);
   virtual ~DrawLayer();

   virtual void Initialize() override;
   virtual void Render(const QMapLibre::CustomLayerRenderParameters&) override;
   virtual void Deinitialize() override;

   virtual bool
   RunMousePicking(const QMapLibre::CustomLayerRenderParameters& params,
                   const QPointF&                                mouseLocalPos,
                   const QPointF&                                mouseGlobalPos,
                   const glm::vec2&                              mouseCoords,
                   const common::Coordinate&                     mouseGeoCoords,
                   std::shared_ptr<types::EventHandler>& eventHandler) override;

protected:
   void AddDrawItem(const std::shared_ptr<gl::draw::DrawItem>& drawItem);
   void ImGuiFrameStart();
   void ImGuiFrameEnd();
   void ImGuiInitialize();
   void
   RenderWithoutImGui(const QMapLibre::CustomLayerRenderParameters& params);
   void ImGuiSelectContext();

private:
   std::unique_ptr<DrawLayerImpl> p;
};

} // namespace scwx::qt::map
