#include <scwx/qt/gl/draw/draw_item.hpp>
#include <scwx/qt/util/maplibre.hpp>

#include <string>

#if defined(_MSC_VER)
#   pragma warning(push, 0)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mbgl/util/constants.hpp>

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

namespace scwx
{
namespace qt
{
namespace gl
{
namespace draw
{

static const std::string logPrefix_ = "scwx::qt::gl::draw::draw_item";

class DrawItem::Impl
{
public:
   explicit Impl() = default;
   ~Impl()         = default;
};

DrawItem::DrawItem() : p(std::make_unique<Impl>()) {}
DrawItem::~DrawItem() = default;

DrawItem::DrawItem(DrawItem&&) noexcept            = default;
DrawItem& DrawItem::operator=(DrawItem&&) noexcept = default;

void DrawItem::Render(
   const QMapLibre::CustomLayerRenderParameters& /* params */)
{
}

void DrawItem::Render(const QMapLibre::CustomLayerRenderParameters& params,
                      bool /* textureAtlasChanged */)
{
   Render(params);
}

bool DrawItem::RunMousePicking(
   const QMapLibre::CustomLayerRenderParameters& /* params */,
   const QPointF& /* mouseLocalPos */,
   const QPointF& /* mouseGlobalPos */,
   const glm::vec2& /* mouseCoords */,
   const common::Coordinate& /* mouseGeoCoords */,
   std::shared_ptr<types::EventHandler>& /* eventHandler */)
{
   // By default, the draw item is not picked
   return false;
}

void DrawItem::UseDefaultProjection(
   const QMapLibre::CustomLayerRenderParameters& params,
   GLint                                         uMVPMatrixLocation)
{
   glm::mat4 projection = glm::ortho(0.0f,
                                     static_cast<float>(params.width),
                                     0.0f,
                                     static_cast<float>(params.height));

   glUniformMatrix4fv(
      uMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

void DrawItem::UseRotationProjection(
   const QMapLibre::CustomLayerRenderParameters& params,
   GLint                                         uMVPMatrixLocation)
{
   glm::mat4 projection = glm::ortho(0.0f,
                                     static_cast<float>(params.width),
                                     0.0f,
                                     static_cast<float>(params.height));

   projection = glm::rotate(projection,
                            glm::radians<float>(params.bearing),
                            glm::vec3(0.0f, 0.0f, 1.0f));

   glUniformMatrix4fv(
      uMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

void DrawItem::UseMapProjection(
   const QMapLibre::CustomLayerRenderParameters& params,
   GLint                                         uMVPMatrixLocation,
   GLint                                         uMapScreenCoordLocation)
{
   const glm::mat4 uMVPMatrix = util::maplibre::GetMapMatrix(params);

   glUniform2fv(uMapScreenCoordLocation,
                1,
                glm::value_ptr(util::maplibre::LatLongToScreenCoordinate(
                   {params.latitude, params.longitude})));

   glUniformMatrix4fv(
      uMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(uMVPMatrix));
}

} // namespace draw
} // namespace gl
} // namespace qt
} // namespace scwx
