#include <scwx/qt/map/radar_site_layer.hpp>
#include <scwx/qt/config/radar_site.hpp>
#include <scwx/qt/gl/draw/geo_lines.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/settings/text_settings.hpp>
#include <scwx/qt/util/maplibre.hpp>
#include <scwx/qt/util/tooltip.hpp>
#include <scwx/common/geographic.hpp>
#include <scwx/util/logger.hpp>

#include <imgui.h>
#include <mbgl/util/constants.hpp>

#include <QGuiApplication>

namespace scwx
{
namespace qt
{
namespace map
{

static const std::string logPrefix_ = "scwx::qt::map::radar_site_layer";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class RadarSiteLayer::Impl
{
public:
   explicit Impl(RadarSiteLayer* self, std::shared_ptr<MapContext>& context) :
       self_ {self}, geoLines_ {std::make_shared<gl::draw::GeoLines>(context)}
   {
      geoLines_->StartLines();
      radarSiteLines_[0] = geoLines_->AddLine();
      radarSiteLines_[1] = geoLines_->AddLine();
      geoLines_->FinishLines();

      static const boost::gil::rgba32f_pixel_t color0 {0.0f, 0.0f, 0.0f, 1.0f};
      static const boost::gil::rgba32f_pixel_t color1 {1.0f, 1.0f, 1.0f, 1.0f};
      static const float                       width = 1;
      geoLines_->SetLineModulate(radarSiteLines_[0], color0);
      geoLines_->SetLineWidth(radarSiteLines_[0], width + 2);

      geoLines_->SetLineModulate(radarSiteLines_[1], color1);
      geoLines_->SetLineWidth(radarSiteLines_[1], width);

      self_->AddDrawItem(geoLines_);
      geoLines_->set_thresholded(false);
   }
   ~Impl() = default;

   void RenderRadarSite(const QMapLibre::CustomLayerRenderParameters& params,
                        std::shared_ptr<config::RadarSite>& radarSite);
   void RenderRadarLine();

   RadarSiteLayer* self_;

   std::vector<std::shared_ptr<config::RadarSite>> radarSites_ {};

   glm::vec2 mapScreenCoordLocation_ {};
   float     mapScale_ {1.0f};
   float     mapBearingCos_ {1.0f};
   float     mapBearingSin_ {0.0f};
   float     halfWidth_ {};
   float     halfHeight_ {};

   std::string hoverText_ {};

   std::shared_ptr<gl::draw::GeoLines>                       geoLines_;
   std::array<std::shared_ptr<gl::draw::GeoLineDrawItem>, 2> radarSiteLines_;
};

RadarSiteLayer::RadarSiteLayer(std::shared_ptr<MapContext> context) :
    DrawLayer(context, "RadarSiteLayer"),
    p(std::make_unique<Impl>(this, context))
{
}

RadarSiteLayer::~RadarSiteLayer() = default;

void RadarSiteLayer::Initialize()
{
   logger_->debug("Initialize()");

   p->radarSites_ = config::RadarSite::GetAll();

   DrawLayer::Initialize();
}

void RadarSiteLayer::Render(
   const QMapLibre::CustomLayerRenderParameters& params)
{
   p->hoverText_.clear();

   auto mapDistance = util::maplibre::GetMapDistance(params);
   auto threshold   = units::length::kilometers<double>(
      settings::GeneralSettings::Instance().radar_site_threshold().GetValue());

   if (!(threshold.value() == 0.0 || mapDistance <= threshold ||
         (threshold.value() < 0 && mapDistance >= -threshold)))
   {
      return;
   }

   gl::OpenGLFunctions& gl = context()->gl();

   // Update map screen coordinate and scale information
   p->mapScreenCoordLocation_ = util::maplibre::LatLongToScreenCoordinate(
      {params.latitude, params.longitude});
   p->mapScale_ = std::pow(2.0, params.zoom) * mbgl::util::tileSize_D /
                  mbgl::util::DEGREES_MAX;
   p->mapBearingCos_ = cosf(params.bearing * common::kDegreesToRadians);
   p->mapBearingSin_ = sinf(params.bearing * common::kDegreesToRadians);
   p->halfWidth_     = params.width * 0.5f;
   p->halfHeight_    = params.height * 0.5f;

   ImGuiFrameStart();
   // Radar site ImGui windows shouldn't have padding
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {0.0f, 0.0f});

   for (auto& radarSite : p->radarSites_)
   {
      p->RenderRadarSite(params, radarSite);
   }

   ImGui::PopStyleVar();

   p->RenderRadarLine();

   DrawLayer::RenderWithoutImGui(params);

   ImGuiFrameEnd();
   SCWX_GL_CHECK_ERROR();
}

void RadarSiteLayer::Impl::RenderRadarSite(
   const QMapLibre::CustomLayerRenderParameters& params,
   std::shared_ptr<config::RadarSite>&           radarSite)
{
   const std::string windowName = fmt::format("radar-site-{}", radarSite->id());

   const auto screenCoordinates =
      (util::maplibre::LatLongToScreenCoordinate(
          {radarSite->latitude(), radarSite->longitude()}) -
       mapScreenCoordLocation_) *
      mapScale_;

   // Rotate text according to map rotation
   float rotatedX = screenCoordinates.x;
   float rotatedY = screenCoordinates.y;
   if (params.bearing != 0.0)
   {
      rotatedX = screenCoordinates.x * mapBearingCos_ -
                 screenCoordinates.y * mapBearingSin_;
      rotatedY = screenCoordinates.x * mapBearingSin_ +
                 screenCoordinates.y * mapBearingCos_;
   }

   // Convert screen to ImGui coordinates
   float x = rotatedX + halfWidth_;
   float y = params.height - (rotatedY + halfHeight_);

   // Setup window to hold text
   ImGui::SetNextWindowPos(
      ImVec2 {x, y}, ImGuiCond_Always, ImVec2 {0.5f, 0.5f});
   if (ImGui::Begin(windowName.c_str(),
                    nullptr,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_AlwaysAutoResize))
   {
      // Render text
      if (ImGui::Button(radarSite->id().c_str()))
      {
         Q_EMIT self_->RadarSiteSelected(radarSite->id());
         self_->ImGuiSelectContext();
      }

      // Store hover text for mouse picking pass
      if (settings::TextSettings::Instance()
             .radar_site_hover_text_enabled()
             .GetValue() &&
          ImGui::IsItemHovered())
      {
         hoverText_ =
            fmt::format("{} ({})\n{}\n{}, {}",
                        radarSite->id(),
                        radarSite->type_name(),
                        radarSite->location_name(),
                        common::GetLatitudeString(radarSite->latitude()),
                        common::GetLongitudeString(radarSite->longitude()));
      }

      // End window
      ImGui::End();
   }
}

void RadarSiteLayer::Impl::RenderRadarLine()
{
   // TODO check if state is updated.
   if ((QGuiApplication::keyboardModifiers() &
        Qt::KeyboardModifier::ShiftModifier) &&
       self_->context()->radar_site() != nullptr)
   {
      const auto&  mouseCoord     = self_->context()->mouse_coordinate();
      const double radarLatitude  = self_->context()->radar_site()->latitude();
      const double radarLongitude = self_->context()->radar_site()->longitude();

      geoLines_->SetLineLocation(radarSiteLines_[0],
                                 static_cast<float>(mouseCoord.latitude_),
                                 static_cast<float>(mouseCoord.longitude_),
                                 static_cast<float>(radarLatitude),
                                 static_cast<float>(radarLongitude));
      geoLines_->SetLineVisible(radarSiteLines_[0], true);

      geoLines_->SetLineLocation(radarSiteLines_[1],
                                 static_cast<float>(mouseCoord.latitude_),
                                 static_cast<float>(mouseCoord.longitude_),
                                 static_cast<float>(radarLatitude),
                                 static_cast<float>(radarLongitude));
      geoLines_->SetLineVisible(radarSiteLines_[1], true);
   }
   else
   {
      geoLines_->SetLineVisible(radarSiteLines_[0], false);
      geoLines_->SetLineVisible(radarSiteLines_[1], false);
   }
}

void RadarSiteLayer::Deinitialize()
{
   logger_->debug("Deinitialize()");

   p->radarSites_.clear();
}

bool RadarSiteLayer::RunMousePicking(
   const QMapLibre::CustomLayerRenderParameters& /* params */,
   const QPointF& /* mouseLocalPos */,
   const QPointF& mouseGlobalPos,
   const glm::vec2& /* mouseCoords */,
   const common::Coordinate& /* mouseGeoCoords */,
   std::shared_ptr<types::EventHandler>& /* eventHandler */)
{
   if (!p->hoverText_.empty())
   {
      util::tooltip::Show(p->hoverText_, mouseGlobalPos);
      return true;
   }

   return false;
}

} // namespace map
} // namespace qt
} // namespace scwx
