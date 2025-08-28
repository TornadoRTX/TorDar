#include <scwx/common/characters.hpp>
#include <scwx/qt/gl/draw/geo_icons.hpp>
#include <scwx/qt/gl/draw/icons.hpp>
#include <scwx/qt/gl/draw/rectangle.hpp>
#include <scwx/qt/manager/font_manager.hpp>
#include <scwx/qt/manager/position_manager.hpp>
#include <scwx/qt/manager/resource_manager.hpp>
#include <scwx/qt/map/map_settings.hpp>
#include <scwx/qt/map/overlay_layer.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/types/texture_types.hpp>
#include <scwx/qt/view/radar_product_view.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/time.hpp>

#if defined(_MSC_VER)
#   pragma warning(push, 0)
#endif

#include <imgui.h>
#include <QGeoPositionInfo>
#include <QGuiApplication>
#include <QMouseEvent>

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

namespace scwx::qt::map
{

static const std::string logPrefix_ = "scwx::qt::map::overlay_layer";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class OverlayLayer::Impl
{
public:
   explicit Impl(OverlayLayer*                         self,
                 const std::shared_ptr<gl::GlContext>& glContext) :
       self_ {self},
       activeBoxOuter_ {std::make_shared<gl::draw::Rectangle>(glContext)},
       activeBoxInner_ {std::make_shared<gl::draw::Rectangle>(glContext)},
       geoIcons_ {std::make_shared<gl::draw::GeoIcons>(glContext)},
       icons_ {std::make_shared<gl::draw::Icons>(glContext)},
       renderMutex_ {}
   {
      auto& generalSettings = settings::GeneralSettings::Instance();

      clockFormatCallbackUuid_ =
         generalSettings.clock_format().RegisterValueChangedCallback(
            [this](const std::string&)
            {
               sweepTimeNeedsUpdate_ = true;
               Q_EMIT self_->NeedsRendering();
            });
      defaultTimeZoneCallbackUuid_ =
         generalSettings.default_time_zone().RegisterValueChangedCallback(
            [this](const std::string&)
            {
               sweepTimeNeedsUpdate_ = true;
               Q_EMIT self_->NeedsRendering();
            });
      showMapAttributionCallbackUuid_ =
         generalSettings.show_map_attribution().RegisterValueChangedCallback(
            [this](const bool&) { Q_EMIT self_->NeedsRendering(); });
      showMapCenterCallbackUuid_ =
         generalSettings.show_map_center().RegisterValueChangedCallback(
            [this](const bool&) { Q_EMIT self_->NeedsRendering(); });
      showMapLogoCallbackUuid_ =
         generalSettings.show_map_logo().RegisterValueChangedCallback(
            [this](const bool&) { Q_EMIT self_->NeedsRendering(); });
   }

   ~Impl()
   {
      auto& generalSettings = settings::GeneralSettings::Instance();

      generalSettings.clock_format().UnregisterValueChangedCallback(
         clockFormatCallbackUuid_);
      generalSettings.default_time_zone().UnregisterValueChangedCallback(
         defaultTimeZoneCallbackUuid_);
      generalSettings.show_map_attribution().UnregisterValueChangedCallback(
         showMapAttributionCallbackUuid_);
      generalSettings.show_map_center().UnregisterValueChangedCallback(
         showMapCenterCallbackUuid_);
      generalSettings.show_map_logo().UnregisterValueChangedCallback(
         showMapLogoCallbackUuid_);
   }

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   void SetupGeoIcons();
   void SetCusorLocation(common::Coordinate coordinate);

   OverlayLayer* self_;

   boost::uuids::uuid clockFormatCallbackUuid_;
   boost::uuids::uuid defaultTimeZoneCallbackUuid_;
   boost::uuids::uuid showMapAttributionCallbackUuid_;
   boost::uuids::uuid showMapCenterCallbackUuid_;
   boost::uuids::uuid showMapLogoCallbackUuid_;

   std::shared_ptr<manager::PositionManager> positionManager_ {
      manager::PositionManager::Instance()};
   QGeoPositionInfo currentPosition_ {};

   std::shared_ptr<gl::draw::Rectangle> activeBoxOuter_;
   std::shared_ptr<gl::draw::Rectangle> activeBoxInner_;
   std::shared_ptr<gl::draw::GeoIcons>  geoIcons_;
   std::shared_ptr<gl::draw::Icons>     icons_;

   const std::string& locationIconName_ {
      types::GetTextureName(types::ImageTexture::Crosshairs24)};
   std::shared_ptr<gl::draw::GeoIconDrawItem> locationIcon_ {};

   std::shared_ptr<gl::draw::GeoIconDrawItem> cursorIcon_ {};

   const std::string& cardinalPointIconName_ {
      types::GetTextureName(types::ImageTexture::CardinalPoint24)};
   const std::string& compassIconName_ {
      types::GetTextureName(types::ImageTexture::Compass24)};
   std::string cursorIconName_ {
      types::GetTextureName(types::ImageTexture::Dot3)};
   const std::string& mapCenterIconName_ {
      types::GetTextureName(types::ImageTexture::Cursor17)};

   std::shared_ptr<boost::gil::rgba8_image_t> cursorIconImage_ {nullptr};

   const std::string& mapboxLogoImageName_ {
      types::GetTextureName(types::ImageTexture::MapboxLogo)};
   const std::string& mapTilerLogoImageName_ {
      types::GetTextureName(types::ImageTexture::MapTilerLogo)};

   std::shared_ptr<gl::draw::IconDrawItem> compassIcon_ {};
   std::shared_ptr<gl::draw::IconDrawItem> mapCenterIcon_ {};
   double                                  lastBearing_ {0.0};

   std::shared_ptr<gl::draw::IconDrawItem> mapLogoIcon_ {};

   bool     firstRender_ {true};
   double   lastWidth_ {0.0};
   double   lastHeight_ {0.0};
   float    lastFontSize_ {0.0f};
   QMargins lastColorTableMargins_ {};

   double                             cursorScale_ {1};
   boost::signals2::scoped_connection cursorScaleConnection_;

   std::mutex renderMutex_;

   std::string sweepTimeString_ {};
   bool        sweepTimeNeedsUpdate_ {true};
   bool        sweepTimePicked_ {false};
};

OverlayLayer::OverlayLayer(const std::shared_ptr<gl::GlContext>& glContext) :
    DrawLayer(glContext, "OverlayLayer"),
    p(std::make_unique<Impl>(this, glContext))
{
   AddDrawItem(p->activeBoxOuter_);
   AddDrawItem(p->activeBoxInner_);
   AddDrawItem(p->geoIcons_);
   AddDrawItem(p->icons_);

   p->activeBoxOuter_->SetPosition(0.0f, 0.0f);
}

OverlayLayer::~OverlayLayer()
{
   p->cursorScaleConnection_.disconnect();
}

void OverlayLayer::Impl::SetCusorLocation(common::Coordinate coordinate)
{
   geoIcons_->SetIconLocation(
      cursorIcon_, coordinate.latitude_, coordinate.longitude_);
}

void OverlayLayer::Impl::SetupGeoIcons()
{
   const std::unique_lock lock {renderMutex_};

   auto& generalSettings = settings::GeneralSettings::Instance();
   cursorScale_          = generalSettings.cursor_icon_scale().GetValue();

   const std::string& texturePath =
      types::GetTexturePath(types::ImageTexture::Dot3);
   cursorIconName_ = fmt::format(
      "{}x{}", types::GetTextureName(types::ImageTexture::Dot3), cursorScale_);
   cursorIconImage_ = manager::ResourceManager::LoadImageResource(
      texturePath, cursorIconName_, cursorScale_);
   manager::ResourceManager::BuildAtlas();

   auto coordinate = currentPosition_.coordinate();
   geoIcons_->StartIconSheets();
   geoIcons_->AddIconSheet(cursorIconName_);
   geoIcons_->AddIconSheet(locationIconName_);
   geoIcons_->FinishIconSheets();

   geoIcons_->StartIcons();

   cursorIcon_ = geoIcons_->AddIcon();
   geoIcons_->SetIconTexture(cursorIcon_, cursorIconName_, 0);

   locationIcon_ = geoIcons_->AddIcon();
   geoIcons_->SetIconTexture(locationIcon_, locationIconName_, 0);
   geoIcons_->SetIconLocation(
      locationIcon_, coordinate.latitude(), coordinate.longitude());

   geoIcons_->FinishIcons();
}

void OverlayLayer::Initialize(const std::shared_ptr<MapContext>& mapContext)
{
   logger_->debug("Initialize()");

   DrawLayer::Initialize(mapContext);

   auto radarProductView = mapContext->radar_product_view();

   if (radarProductView != nullptr)
   {
      connect(radarProductView.get(),
              &view::RadarProductView::SweepComputed,
              this,
              &OverlayLayer::UpdateSweepTimeNextFrame);
   }

   p->currentPosition_ = p->positionManager_->position();

   // Geo Icons
   auto& generalSettings = settings::GeneralSettings::Instance();
   p->SetupGeoIcons();
   p->cursorScaleConnection_ =
      generalSettings.cursor_icon_scale().changed_signal().connect(
         [this]()
         {
            p->SetupGeoIcons();
            Q_EMIT NeedsRendering();
         });

   // Icons
   p->icons_->StartIconSheets();
   p->icons_->AddIconSheet(p->cardinalPointIconName_);
   p->icons_->AddIconSheet(p->compassIconName_);
   p->icons_->AddIconSheet(p->mapCenterIconName_);
   p->icons_->AddIconSheet(p->mapboxLogoImageName_)->SetAnchor(0.0f, 1.0f);
   p->icons_->AddIconSheet(p->mapTilerLogoImageName_)->SetAnchor(0.0f, 1.0f);
   p->icons_->FinishIconSheets();

   p->icons_->StartIcons();
   p->compassIcon_ = p->icons_->AddIcon();
   p->icons_->SetIconTexture(p->compassIcon_, p->cardinalPointIconName_, 0);
   gl::draw::Icons::RegisterEventHandler(
      p->compassIcon_,
      [this, mapContext](QEvent* ev)
      {
         switch (ev->type())
         {
         case QEvent::Type::Enter:
            // Highlight icon on mouse enter
            p->icons_->SetIconModulate(
               p->compassIcon_,
               boost::gil::rgba32f_pixel_t {1.5f, 1.5f, 1.5f, 1.0f});
            break;

         case QEvent::Type::Leave:
            // Restore icon on mouse leave
            p->icons_->SetIconModulate(
               p->compassIcon_,
               boost::gil::rgba32f_pixel_t {1.0f, 1.0f, 1.0f, 1.0f});
            break;

         case QEvent::Type::MouseButtonPress:
         {
            // Reset bearing on mouse button press
            QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(ev);
            if (mouseEvent->buttons() == Qt::MouseButton::LeftButton &&
                p->lastBearing_ != 0.0)
            {
               auto map = mapContext->map().lock();
               if (map != nullptr)
               {
                  map->setBearing(0.0);
               }
               ev->accept();
            }
            break;
         }

         default:
            break;
         }
      });

   p->mapCenterIcon_ = p->icons_->AddIcon();
   p->icons_->SetIconTexture(p->mapCenterIcon_, p->mapCenterIconName_, 0);

   p->mapLogoIcon_ = p->icons_->AddIcon();
   if (mapContext->map_provider() == MapProvider::Mapbox)
   {
      p->icons_->SetIconTexture(p->mapLogoIcon_, p->mapboxLogoImageName_, 0);
   }
   else if (mapContext->map_provider() == MapProvider::MapTiler)
   {
      p->icons_->SetIconTexture(p->mapLogoIcon_, p->mapTilerLogoImageName_, 0);
   }

   p->icons_->FinishIcons();

   connect(p->positionManager_.get(),
           &manager::PositionManager::LocationTrackingChanged,
           this,
           [this]() { Q_EMIT NeedsRendering(); });
   connect(p->positionManager_.get(),
           &manager::PositionManager::PositionUpdated,
           this,
           [this](const QGeoPositionInfo& position)
           {
              auto coordinate = position.coordinate();
              if (position.isValid() &&
                  p->currentPosition_.coordinate() != coordinate)
              {
                 p->geoIcons_->SetIconLocation(p->locationIcon_,
                                               coordinate.latitude(),
                                               coordinate.longitude());
                 Q_EMIT NeedsRendering();
              }
              p->currentPosition_ = position;
           });
}

void OverlayLayer::Render(const std::shared_ptr<MapContext>& mapContext,
                          const QMapLibre::CustomLayerRenderParameters& params)
{
   const std::unique_lock lock {p->renderMutex_};

   auto        radarProductView = mapContext->radar_product_view();
   auto&       settings         = mapContext->settings();
   const float pixelRatio       = mapContext->pixel_ratio();

   ImGuiFrameStart(mapContext);

   p->sweepTimePicked_ = false;

   if (radarProductView != nullptr)
   {
      scwx::util::ClockFormat clockFormat = scwx::util::GetClockFormat(
         settings::GeneralSettings::Instance().clock_format().GetValue());

      auto radarProductManager = radarProductView->radar_product_manager();

      const scwx::util::time_zone* currentZone =
         (radarProductManager != nullptr) ?
            radarProductManager->default_time_zone() :
            nullptr;

      p->sweepTimeString_ = scwx::util::TimeString(
         radarProductView->sweep_time(), clockFormat, currentZone, false);
      p->sweepTimeNeedsUpdate_ = false;
   }

   // Active Box
   p->activeBoxOuter_->SetVisible(settings.isActive_);
   p->activeBoxInner_->SetVisible(settings.isActive_);
   if (settings.isActive_)
   {
      p->activeBoxOuter_->SetSize(params.width, params.height);
      p->activeBoxInner_->SetSize(params.width - (2.0f * pixelRatio),
                                  params.height - (2.0f * pixelRatio));

      p->activeBoxInner_->SetPosition(1.0f * pixelRatio, 1.0f * pixelRatio);

      p->activeBoxOuter_->SetBorder(1.0f * pixelRatio, {0, 0, 0, 255});
      p->activeBoxInner_->SetBorder(1.0f * pixelRatio, {255, 255, 255, 255});
   }

   auto& generalSettings = settings::GeneralSettings::Instance();

   // Cursor Icon
   bool cursorIconVisible =
      generalSettings.cursor_icon_always_on().GetValue() ||
      (QGuiApplication::keyboardModifiers() &
       Qt::KeyboardModifier::ControlModifier);
   p->geoIcons_->SetIconVisible(p->cursorIcon_, cursorIconVisible);
   if (cursorIconVisible)
   {
      p->SetCusorLocation(mapContext->mouse_coordinate());
   }

   // Location Icon
   p->geoIcons_->SetIconVisible(p->locationIcon_,
                                p->currentPosition_.isValid() &&
                                   p->positionManager_->IsLocationTracked());

   // Compass Icon
   if (params.width != p->lastWidth_ || params.height != p->lastHeight_ ||
       ImGui::GetFontSize() != p->lastFontSize_)
   {
      // Set the compass icon in the upper right, below the sweep time window
      p->icons_->SetIconLocation(p->compassIcon_,
                                 params.width - 24,
                                 params.height - (ImGui::GetFontSize() + 32));
   }
   if (params.bearing != p->lastBearing_)
   {
      if (params.bearing == 0.0)
      {
         // Use cardinal point icon when bearing is oriented north-up
         p->icons_->SetIconTexture(
            p->compassIcon_, p->cardinalPointIconName_, 0);
         p->icons_->SetIconAngle(p->compassIcon_,
                                 units::angle::degrees<double> {0.0});
      }
      else
      {
         // Use rotated compass icon when bearing is rotated away from north-up
         p->icons_->SetIconTexture(p->compassIcon_, p->compassIconName_, 0);
         p->icons_->SetIconAngle(
            p->compassIcon_,
            units::angle::degrees<double> {-45 - params.bearing});
      }
   }

   if (radarProductView != nullptr)
   {
      // Render product name
      const std::string productName = radarProductView->GetRadarProductName();
      const std::optional<float> elevation = radarProductView->elevation();

      if (productName.length() > 0 && !productName.starts_with('?'))
      {
         ImGui::SetNextWindowPos(ImVec2 {0.0f, 0.0f});
         ImGui::Begin("Product Name",
                      nullptr,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_AlwaysAutoResize);

         if (elevation.has_value())
         {
            const std::string elevationString =
               (QString::number(*elevation, 'f', 1) +
                common::Characters::DEGREE)
                  .toStdString();
            ImGui::TextUnformatted(
               fmt::format("{} ({})", productName, elevationString).c_str());
         }
         else
         {
            ImGui::TextUnformatted(productName.c_str());
         }

         ImGui::End();
      }
   }

   if (p->sweepTimeString_.length() > 0)
   {
      // Render time
      ImGui::SetNextWindowPos(ImVec2 {static_cast<float>(params.width), 0.0f},
                              ImGuiCond_Always,
                              ImVec2 {1.0f, 0.0f});
      ImGui::Begin("Sweep Time",
                   nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_AlwaysAutoResize);

      if (radarProductView != nullptr && ImGui::IsWindowHovered())
      {
         // Show a detailed product description when the sweep time is hovered
         p->sweepTimePicked_ = true;

         auto fields = radarProductView->GetDescriptionFields();
         if (fields.empty())
         {
            ImGui::TextUnformatted(p->sweepTimeString_.c_str());
         }
         else
         {
            if (ImGui::BeginTable("Description Fields", 2))
            {
               for (auto& field : fields)
               {
                  ImGui::TableNextRow();
                  ImGui::TableNextColumn();
                  ImGui::TextUnformatted(field.first.c_str());
                  ImGui::TableNextColumn();
                  ImGui::TextUnformatted(field.second.c_str());
               }
               ImGui::EndTable();
            }
         }
      }
      else
      {
         ImGui::TextUnformatted(p->sweepTimeString_.c_str());
      }

      ImGui::End();
   }

   // Map Center Icon
   if (params.width != p->lastWidth_ || params.height != p->lastHeight_)
   {
      // Draw the icon in the center of the widget
      p->icons_->SetIconLocation(
         p->mapCenterIcon_, params.width / 2.0, params.height / 2.0);
   }
   p->icons_->SetIconVisible(p->mapCenterIcon_,
                             generalSettings.show_map_center().GetValue());

   const QMargins colorTableMargins = mapContext->color_table_margins();
   if (colorTableMargins != p->lastColorTableMargins_ || p->firstRender_)
   {
      // Draw map logo with a 10x10 indent from the bottom left
      p->icons_->SetIconLocation(p->mapLogoIcon_,
                                 10 + colorTableMargins.left(),
                                 10 + colorTableMargins.bottom());
   }
   p->icons_->SetIconVisible(p->mapLogoIcon_,
                             generalSettings.show_map_logo().GetValue());

   DrawLayer::RenderWithoutImGui(params);

   auto mapCopyrights = mapContext->map_copyrights();
   if (mapCopyrights.length() > 0 &&
       generalSettings.show_map_attribution().GetValue())
   {
      auto attributionFont = manager::FontManager::Instance().GetImGuiFont(
         types::FontCategory::Attribution);

      ImGui::SetNextWindowPos(ImVec2 {static_cast<float>(params.width),
                                      static_cast<float>(params.height) -
                                         colorTableMargins.bottom()},
                              ImGuiCond_Always,
                              ImVec2 {1.0f, 1.0f});
      ImGui::SetNextWindowBgAlpha(0.5f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {3.0f, 2.0f});
      ImGui::PushFont(attributionFont.first->font(),
                      attributionFont.second.value());
      ImGui::Begin("Attribution",
                   nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::TextUnformatted(mapCopyrights.c_str());
      ImGui::End();
      ImGui::PopFont();
      ImGui::PopStyleVar();
   }

   p->firstRender_           = false;
   p->lastWidth_             = params.width;
   p->lastHeight_            = params.height;
   p->lastBearing_           = params.bearing;
   p->lastFontSize_          = ImGui::GetFontSize();
   p->lastColorTableMargins_ = colorTableMargins;

   ImGuiFrameEnd();

   SCWX_GL_CHECK_ERROR();
}

void OverlayLayer::Deinitialize()
{
   logger_->debug("Deinitialize()");

   DrawLayer::Deinitialize();

   disconnect(this);

   p->locationIcon_ = nullptr;
}

bool OverlayLayer::RunMousePicking(
   const std::shared_ptr<MapContext>&            mapContext,
   const QMapLibre::CustomLayerRenderParameters& params,
   const QPointF&                                mouseLocalPos,
   const QPointF&                                mouseGlobalPos,
   const glm::vec2&                              mouseCoords,
   const common::Coordinate&                     mouseGeoCoords,
   std::shared_ptr<types::EventHandler>&         eventHandler)
{
   // If sweep time was picked, don't process additional items
   if (p->sweepTimePicked_)
   {
      return true;
   }

   return DrawLayer::RunMousePicking(mapContext,
                                     params,
                                     mouseLocalPos,
                                     mouseGlobalPos,
                                     mouseCoords,
                                     mouseGeoCoords,
                                     eventHandler);
}

void OverlayLayer::UpdateSweepTimeNextFrame()
{
   p->sweepTimeNeedsUpdate_ = true;
}

} // namespace scwx::qt::map
