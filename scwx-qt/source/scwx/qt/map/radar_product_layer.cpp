#include <scwx/qt/map/radar_product_layer.hpp>
#include <scwx/qt/map/map_settings.hpp>
#include <scwx/qt/gl/shader_program.hpp>
#include <scwx/qt/settings/unit_settings.hpp>
#include <scwx/qt/types/unit_types.hpp>
#include <scwx/qt/util/geographic_lib.hpp>
#include <scwx/qt/util/maplibre.hpp>
#include <scwx/qt/util/tooltip.hpp>
#include <scwx/qt/view/radar_product_view.hpp>
#include <scwx/util/logger.hpp>

#if defined(_MSC_VER)
#   pragma warning(push, 0)
#endif

#include <boost/algorithm/string.hpp>
#include <boost/timer/timer.hpp>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mbgl/util/constants.hpp>
#include <QGuiApplication>

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

namespace scwx::qt::map
{

static const std::string logPrefix_ = "scwx::qt::map::radar_product_layer";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class RadarProductLayer::Impl
{
public:
   explicit Impl() = default;
   ~Impl()         = default;

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   std::shared_ptr<gl::ShaderProgram> shaderProgram_ {nullptr};

   GLint uMVPMatrixLocation_ {static_cast<GLint>(GL_INVALID_INDEX)};
   GLint uMapScreenCoordLocation_ {static_cast<GLint>(GL_INVALID_INDEX)};
   GLint uDataMomentOffsetLocation_ {static_cast<GLint>(GL_INVALID_INDEX)};
   GLint uDataMomentScaleLocation_ {static_cast<GLint>(GL_INVALID_INDEX)};
   GLint uCFPEnabledLocation_ {static_cast<GLint>(GL_INVALID_INDEX)};
   std::array<GLuint, 3> vbo_ {GL_INVALID_INDEX};
   GLuint                vao_ {GL_INVALID_INDEX};
   GLuint                texture_ {GL_INVALID_INDEX};

   GLsizeiptr numVertices_ {0};

   bool cfpEnabled_ {false};

   bool colorTableNeedsUpdate_ {false};
   bool sweepNeedsUpdate_ {false};
};

RadarProductLayer::RadarProductLayer(std::shared_ptr<gl::GlContext> glContext) :
    GenericLayer(std::move(glContext)), p(std::make_unique<Impl>())
{
}
RadarProductLayer::~RadarProductLayer() = default;

void RadarProductLayer::Initialize(
   const std::shared_ptr<MapContext>& mapContext)
{
   logger_->debug("Initialize()");

   auto glContext = gl_context();

   gl::OpenGLFunctions& gl = glContext->gl();

   // Load and configure radar shader
   p->shaderProgram_ =
      glContext->GetShaderProgram(":/gl/radar.vert", ":/gl/radar.frag");

   p->uMVPMatrixLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_->id(), "uMVPMatrix");
   if (p->uMVPMatrixLocation_ == -1)
   {
      logger_->warn("Could not find uMVPMatrix");
   }

   p->uMapScreenCoordLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_->id(), "uMapScreenCoord");
   if (p->uMapScreenCoordLocation_ == -1)
   {
      logger_->warn("Could not find uMapScreenCoord");
   }

   p->uDataMomentOffsetLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_->id(), "uDataMomentOffset");
   if (p->uDataMomentOffsetLocation_ == -1)
   {
      logger_->warn("Could not find uDataMomentOffset");
   }

   p->uDataMomentScaleLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_->id(), "uDataMomentScale");
   if (p->uDataMomentScaleLocation_ == -1)
   {
      logger_->warn("Could not find uDataMomentScale");
   }

   p->uCFPEnabledLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_->id(), "uCFPEnabled");
   if (p->uCFPEnabledLocation_ == -1)
   {
      logger_->warn("Could not find uCFPEnabled");
   }

   p->shaderProgram_->Use();

   // Generate a vertex array object
   gl.glGenVertexArrays(1, &p->vao_);

   // Generate vertex buffer objects
   gl.glGenBuffers(3, p->vbo_.data());

   // Update radar sweep
   p->sweepNeedsUpdate_ = true;
   UpdateSweep(mapContext);

   // Create color table
   gl.glGenTextures(1, &p->texture_);
   p->colorTableNeedsUpdate_ = true;
   UpdateColorTable(mapContext);
   gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

   auto radarProductView = mapContext->radar_product_view();
   connect(radarProductView.get(),
           &view::RadarProductView::ColorTableLutUpdated,
           this,
           [this]() { p->colorTableNeedsUpdate_ = true; });
   connect(radarProductView.get(),
           &view::RadarProductView::SweepComputed,
           this,
           [this]() { p->sweepNeedsUpdate_ = true; });
}

void RadarProductLayer::UpdateSweep(
   const std::shared_ptr<MapContext>& mapContext)
{
   gl::OpenGLFunctions& gl = gl_context()->gl();

   boost::timer::cpu_timer timer;

   std::shared_ptr<view::RadarProductView> radarProductView =
      mapContext->radar_product_view();

   std::unique_lock sweepLock(radarProductView->sweep_mutex(),
                              std::try_to_lock);
   if (!sweepLock.owns_lock())
   {
      logger_->trace("Sweep locked, deferring update");
      return;
   }
   logger_->debug("UpdateSweep()");

   p->sweepNeedsUpdate_ = false;

   const std::vector<float>& vertices = radarProductView->vertices();

   // Bind a vertex array object
   gl.glBindVertexArray(p->vao_);

   // Buffer vertices
   gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_[0]);
   timer.start();
   gl.glBufferData(GL_ARRAY_BUFFER,
                   vertices.size() * sizeof(GLfloat),
                   vertices.data(),
                   GL_STATIC_DRAW);
   timer.stop();
   logger_->debug("Vertices buffered in {}", timer.format(6, "%ws"));

   gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
   gl.glEnableVertexAttribArray(0);

   // Buffer data moments
   const GLvoid* data;
   GLsizeiptr    dataSize;
   size_t        componentSize;
   GLenum        type;

   std::tie(data, dataSize, componentSize) = radarProductView->GetMomentData();

   if (componentSize == 1)
   {
      type = GL_UNSIGNED_BYTE;
   }
   else
   {
      type = GL_UNSIGNED_SHORT;
   }

   gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_[1]);
   timer.start();
   gl.glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
   timer.stop();
   logger_->debug("Data moments buffered in {}", timer.format(6, "%ws"));

   gl.glVertexAttribIPointer(1, 1, type, 0, static_cast<void*>(0));
   gl.glEnableVertexAttribArray(1);

   // Buffer CFP data
   const GLvoid* cfpData;
   GLsizeiptr    cfpDataSize;
   size_t        cfpComponentSize;
   GLenum        cfpType;

   std::tie(cfpData, cfpDataSize, cfpComponentSize) =
      radarProductView->GetCfpMomentData();

   if (cfpData != nullptr)
   {
      if (cfpComponentSize == 1)
      {
         cfpType = GL_UNSIGNED_BYTE;
      }
      else
      {
         cfpType = GL_UNSIGNED_SHORT;
      }

      gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_[2]);
      timer.start();
      gl.glBufferData(GL_ARRAY_BUFFER, cfpDataSize, cfpData, GL_STATIC_DRAW);
      timer.stop();
      logger_->debug("CFP moments buffered in {}", timer.format(6, "%ws"));

      gl.glVertexAttribIPointer(2, 1, cfpType, 0, static_cast<void*>(0));
      gl.glEnableVertexAttribArray(2);
   }
   else
   {
      gl.glDisableVertexAttribArray(2);
   }

   p->numVertices_ = vertices.size() / 2;
}

void RadarProductLayer::Render(
   const std::shared_ptr<MapContext>&            mapContext,
   const QMapLibre::CustomLayerRenderParameters& params)
{
   gl::OpenGLFunctions& gl = gl_context()->gl();

   p->shaderProgram_->Use();

   // Set OpenGL blend mode for transparency
   gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   const bool wireframeEnabled = mapContext->settings().radarWireframeEnabled_;
   if (wireframeEnabled)
   {
      // Set polygon mode to draw wireframe
      gl.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }

   if (p->colorTableNeedsUpdate_)
   {
      UpdateColorTable(mapContext);
   }

   if (p->sweepNeedsUpdate_)
   {
      UpdateSweep(mapContext);
   }

   const float scale = std::pow(2.0, params.zoom) * 2.0f *
                       mbgl::util::tileSize_D / mbgl::util::DEGREES_MAX;
   const float xScale = scale / params.width;
   const float yScale = scale / params.height;

   glm::mat4 uMVPMatrix(1.0f);
   uMVPMatrix = glm::scale(uMVPMatrix, glm::vec3(xScale, yScale, 1.0f));
   uMVPMatrix = glm::rotate(uMVPMatrix,
                            glm::radians<float>(params.bearing),
                            glm::vec3(0.0f, 0.0f, 1.0f));

   gl.glUniform2fv(p->uMapScreenCoordLocation_,
                   1,
                   glm::value_ptr(util::maplibre::LatLongToScreenCoordinate(
                      {params.latitude, params.longitude})));

   gl.glUniformMatrix4fv(
      p->uMVPMatrixLocation_, 1, GL_FALSE, glm::value_ptr(uMVPMatrix));

   gl.glUniform1i(p->uCFPEnabledLocation_, p->cfpEnabled_ ? 1 : 0);

   gl.glActiveTexture(GL_TEXTURE0);
   gl.glBindTexture(GL_TEXTURE_1D, p->texture_);
   gl.glBindVertexArray(p->vao_);
   gl.glDrawArrays(GL_TRIANGLES, 0, p->numVertices_);

   if (wireframeEnabled)
   {
      // Restore polygon mode to default
      gl.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }

   SCWX_GL_CHECK_ERROR();
}

void RadarProductLayer::Deinitialize()
{
   logger_->debug("Deinitialize()");

   gl::OpenGLFunctions& gl = gl_context()->gl();

   gl.glDeleteVertexArrays(1, &p->vao_);
   gl.glDeleteBuffers(3, p->vbo_.data());

   p->uMVPMatrixLocation_        = GL_INVALID_INDEX;
   p->uMapScreenCoordLocation_   = GL_INVALID_INDEX;
   p->uDataMomentOffsetLocation_ = GL_INVALID_INDEX;
   p->uDataMomentScaleLocation_  = GL_INVALID_INDEX;
   p->uCFPEnabledLocation_       = GL_INVALID_INDEX;
   p->vao_                       = GL_INVALID_INDEX;
   p->vbo_                       = {GL_INVALID_INDEX};
   p->texture_                   = GL_INVALID_INDEX;
}

bool RadarProductLayer::RunMousePicking(
   const std::shared_ptr<MapContext>& mapContext,
   const QMapLibre::CustomLayerRenderParameters& /* params */,
   const QPointF& /* mouseLocalPos */,
   const QPointF& mouseGlobalPos,
   const glm::vec2& /* mouseCoords */,
   const common::Coordinate& mouseGeoCoords,
   std::shared_ptr<types::EventHandler>& /* eventHandler */)
{
   bool itemPicked = false;

   if (QGuiApplication::keyboardModifiers() &
       Qt::KeyboardModifier::ShiftModifier)
   {
      std::shared_ptr<view::RadarProductView> radarProductView =
         mapContext->radar_product_view();

      if (mapContext->radar_site() == nullptr)
      {
         return itemPicked;
      }

      // Get distance and altitude of point
      const double radarLatitude  = mapContext->radar_site()->latitude();
      const double radarLongitude = mapContext->radar_site()->longitude();

      const auto distanceMeters =
         util::GeographicLib::GetDistance(mouseGeoCoords.latitude_,
                                          mouseGeoCoords.longitude_,
                                          radarLatitude,
                                          radarLongitude);

      const std::string distanceUnitName =
         settings::UnitSettings::Instance().distance_units().GetValue();
      const types::DistanceUnits distanceUnits =
         types::GetDistanceUnitsFromName(distanceUnitName);
      const double distanceScale = types::GetDistanceUnitsScale(distanceUnits);
      const std::string distanceAbbrev =
         types::GetDistanceUnitsAbbreviation(distanceUnits);

      const double distance = distanceMeters.value() *
                              scwx::common::kKilometersPerMeter * distanceScale;
      std::string distanceHeightStr =
         fmt::format("{:.2f} {}", distance, distanceAbbrev);

      if (radarProductView == nullptr)
      {
         util::tooltip::Show(distanceHeightStr, mouseGlobalPos);
         itemPicked = true;
         return itemPicked;
      }

      std::optional<float> elevation = radarProductView->elevation();
      if (elevation.has_value())
      {
         const auto altitudeMeters =
            util::GeographicLib::GetRadarBeamAltititude(
               distanceMeters,
               units::angle::degrees<double>(*elevation),
               mapContext->radar_site()->altitude());

         const std::string heightUnitName =
            settings::UnitSettings::Instance().echo_tops_units().GetValue();
         const types::EchoTopsUnits heightUnits =
            types::GetEchoTopsUnitsFromName(heightUnitName);
         const double heightScale = types::GetEchoTopsUnitsScale(heightUnits);
         const std::string heightAbbrev =
            types::GetEchoTopsUnitsAbbreviation(heightUnits);

         const double altitude = altitudeMeters.value() *
                                 scwx::common::kKilometersPerMeter *
                                 heightScale;

         distanceHeightStr = fmt::format(
            "{}\n{:.2f} {}", distanceHeightStr, altitude, heightAbbrev);
      }

      std::optional<std::uint16_t> binLevel =
         radarProductView->GetBinLevel(mouseGeoCoords);

      if (binLevel.has_value())
      {
         // Hovering over a bin on the map
         std::optional<wsr88d::DataLevelCode> code =
            radarProductView->GetDataLevelCode(binLevel.value());
         std::optional<float> value =
            radarProductView->GetDataValue(binLevel.value());

         if (code.has_value() && //
             code.value() != wsr88d::DataLevelCode::Blank &&
             code.value() != wsr88d::DataLevelCode::NoData &&
             code.value() != wsr88d::DataLevelCode::Topped)
         {
            // Level has associated data level code
            std::string codeName = wsr88d::GetDataLevelCodeName(code.value());
            std::string codeShortName =
               wsr88d::GetDataLevelCodeShortName(code.value());
            std::string hoverText;

            if (codeName != codeShortName && !codeShortName.empty())
            {
               // There is a unique long and short name for the code
               hoverText = fmt::format(
                  "{}: {}\n{}", codeShortName, codeName, distanceHeightStr);
            }
            else
            {
               // Otherwise, only use the long name (always present)
               hoverText = fmt::format("{}\n{}", codeName, distanceHeightStr);
            }

            // Show the tooltip
            util::tooltip::Show(hoverText, mouseGlobalPos);

            itemPicked = true;
         }
         else if (value.has_value())
         {
            // Level has associated data value
            float       f = value.value();
            std::string units {};
            std::string suffix {};
            std::string hoverText;

            // Determine units from radar product view
            units = radarProductView->units();
            if (!units.empty())
            {
               f = f * radarProductView->unit_scale();
            }
            else
            {
               std::shared_ptr<common::ColorTable> colorTable =
                  radarProductView->color_table();

               if (colorTable != nullptr)
               {
                  // Scale data value according to the color table, and get
                  // units
                  f     = f * colorTable->scale() + colorTable->offset();
                  units = colorTable->units();
               }
            }

            if (code.has_value() &&
                code.value() == wsr88d::DataLevelCode::Topped)
            {
               // Show " TOPPED" suffix for echo tops
               suffix = " TOPPED";
            }

            if (units.empty() ||          //
                units.starts_with("?") || //
                boost::iequals(units, "NONE") ||
                boost::iequals(units, "UNITLESS") ||
                radarProductView->IgnoreUnits())
            {
               // Don't display a units value that wasn't intended to be
               // displayed
               hoverText =
                  fmt::format("{}{}\n{}", f, suffix, distanceHeightStr);
            }
            else if (std::isalpha(static_cast<unsigned char>(units.at(0))))
            {
               // dBZ, Kts, etc.
               hoverText = fmt::format(
                  "{} {}{}\n{}", f, units, suffix, distanceHeightStr);
            }
            else
            {
               // %, etc.
               hoverText = fmt::format(
                  "{}{}{}\n{}", f, units, suffix, distanceHeightStr);
            }

            // Show the tooltip
            util::tooltip::Show(hoverText, mouseGlobalPos);

            itemPicked = true;
         }
      }
      else
      {
         // Always show tooltip for distance and altitude
         util::tooltip::Show(distanceHeightStr, mouseGlobalPos);
         itemPicked = true;
      }
   }

   return itemPicked;
}

void RadarProductLayer::UpdateColorTable(
   const std::shared_ptr<MapContext>& mapContext)
{
   logger_->debug("UpdateColorTable()");

   p->colorTableNeedsUpdate_ = false;

   gl::OpenGLFunctions&                    gl = gl_context()->gl();
   std::shared_ptr<view::RadarProductView> radarProductView =
      mapContext->radar_product_view();

   const std::vector<boost::gil::rgba8_pixel_t>& colorTable =
      radarProductView->color_table_lut();
   const uint16_t rangeMin = radarProductView->color_table_min();
   const uint16_t rangeMax = radarProductView->color_table_max();

   const float scale = rangeMax - rangeMin;

   gl.glActiveTexture(GL_TEXTURE0);
   gl.glBindTexture(GL_TEXTURE_1D, p->texture_);
   gl.glTexImage1D(GL_TEXTURE_1D,
                   0,
                   GL_RGBA,
                   (GLsizei) colorTable.size(),
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   colorTable.data());
   gl.glGenerateMipmap(GL_TEXTURE_1D);

   gl.glUniform1ui(p->uDataMomentOffsetLocation_, rangeMin);
   gl.glUniform1f(p->uDataMomentScaleLocation_, scale);
}

} // namespace scwx::qt::map
