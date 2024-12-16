#pragma once

#include <scwx/common/geographic.hpp>
#include <scwx/common/products.hpp>
#include <scwx/qt/config/radar_site.hpp>
#include <scwx/qt/types/map_types.hpp>
#include <scwx/qt/types/radar_product_record.hpp>
#include <scwx/qt/types/text_event_key.hpp>

#include <chrono>
#include <memory>

#include <qmaplibre.hpp>

#include <QOpenGLWidget>
#include <QPropertyAnimation>
#include <QtGlobal>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace scwx
{
namespace qt
{
namespace map
{

class MapWidgetImpl;

class MapWidget : public QOpenGLWidget
{
   Q_OBJECT

public:
   explicit MapWidget(std::size_t id, const QMapLibre::Settings&);
   ~MapWidget();

   void DumpLayerList() const;

   [[nodiscard]] common::Level3ProductCategoryMap
                                           GetAvailableLevel3Categories();
   [[nodiscard]] float                     GetElevation() const;
   [[nodiscard]] std::vector<float>        GetElevationCuts() const;
   [[nodiscard]] std::vector<std::string>  GetLevel3Products();
   [[nodiscard]] std::string               GetMapStyle() const;
   [[nodiscard]] common::RadarProductGroup GetRadarProductGroup() const;
   [[nodiscard]] std::string               GetRadarProductName() const;
   [[nodiscard]] std::shared_ptr<config::RadarSite> GetRadarSite() const;
   [[nodiscard]] bool GetRadarWireframeEnabled() const;
   [[nodiscard]] std::chrono::system_clock::time_point GetSelectedTime() const;
   [[nodiscard]] bool          GetSmoothingEnabled() const;
   [[nodiscard]] std::uint16_t GetVcp() const;

   void SelectElevation(float elevation);

   /**
    * @brief Selects a radar product.
    *
    * @param [in] group Radar product group
    * @param [in] product Radar product name
    * @param [in] productCode Radar product code (optional)
    * @param [in] time Product time. Default is the latest available.
    * @param [in] update Whether to update the radar product view on selection
    */
   void SelectRadarProduct(common::RadarProductGroup group,
                           const std::string&        product,
                           std::int16_t              productCode        = 0,
                           std::chrono::system_clock::time_point time   = {},
                           bool                                  update = true);

   void SelectRadarProduct(std::shared_ptr<types::RadarProductRecord> record);

   /**
    * @brief Selects a radar site.
    *
    * @param [in] radarSite ID of the requested radar site
    * @param [in] updateCoordinates Whether to update the map coordinates to the
    * requested radar site location. Default is true.
    */
   void SelectRadarSite(const std::string& id, bool updateCoordinates = true);

   /**
    * @brief Selects a radar site.
    *
    * @param [in] radarSite Shared pointer to the requested radar site
    * @param [in] updateCoordinates Whether to update the map coordinates to the
    * requested radar site location. Default is true.
    */
   void SelectRadarSite(std::shared_ptr<config::RadarSite> radarSite,
                        bool updateCoordinates = true);

   /**
    * @brief Selects the time associated with the active radar product.
    *
    * @param [in] time Product time
    */
   void SelectTime(std::chrono::system_clock::time_point time);

   void SetActive(bool isActive);
   void SetAutoRefresh(bool enabled);
   void SetAutoUpdate(bool enabled);

   /**
    * @brief Sets the current map location.
    *
    * @param [in] latitude Latitude in degrees
    * @param [in] longitude Longitude in degrees
    * @param [in] updateRadarSite Whether to update the selected radar site to
    * the closest WSR-88D site. Default is false.
    */
   void SetMapLocation(double latitude,
                       double longitude,
                       bool   updateRadarSite = false);
   void SetMapParameters(double latitude,
                         double longitude,
                         double zoom,
                         double bearing,
                         double pitch);
   void SetInitialMapStyle(const std::string& styleName);
   void SetMapStyle(const std::string& styleName);
   void SetRadarWireframeEnabled(bool enabled);
   void SetSmoothingEnabled(bool enabled);

   /**
    * Updates the coordinates associated with mouse movement from another map.
    *
    * @param [in] coordinate Coordinate of the mouse
    */
   void UpdateMouseCoordinate(const common::Coordinate& coordinate);

private:
   void  changeStyle();
   qreal pixelRatio();

   // QWidget implementation.
   bool event(QEvent* e) override;
   void enterEvent(QEnterEvent* ev) override final;
   void keyPressEvent(QKeyEvent* ev) override final;
   void keyReleaseEvent(QKeyEvent* ev) override final;
   void leaveEvent(QEvent* ev) override final;
   void mousePressEvent(QMouseEvent* ev) override final;
   void mouseMoveEvent(QMouseEvent* ev) override final;
   void wheelEvent(QWheelEvent* ev) override final;

   // QOpenGLWidget implementation.
   void initializeGL() override final;
   void paintGL() override final;

   std::unique_ptr<MapWidgetImpl> p;

   friend class MapWidgetImpl;

private slots:
   void mapChanged(QMapLibre::Map::MapChange);

signals:
   void AlertSelected(const types::TextEventKey& key);
   void Level3ProductsChanged();
   void MapParametersChanged(double latitude,
                             double longitude,
                             double zoom,
                             double bearing,
                             double pitch);
   void MapStyleChanged(const std::string& styleName);

   /**
    * This signal is emitted when the mouse moves to a different geographic
    * coordinate within the map widget. This signal is emitted during paintGL(),
    * and must be connected using a connection type of queued if the slot
    * triggers a repaint.
    *
    * @param [in] coordinate Geographic coordinate of the mouse
    */
   void MouseCoordinateChanged(common::Coordinate coordinate);

   void RadarSiteRequested(const std::string& id,
                           bool               updateCoordinates = true);
   void RadarSiteUpdated(std::shared_ptr<config::RadarSite> radarSite);
   void RadarSweepUpdated();
   void RadarSweepNotUpdated(types::NoUpdateReason reason);
   void WidgetPainted();
};

} // namespace map
} // namespace qt
} // namespace scwx
