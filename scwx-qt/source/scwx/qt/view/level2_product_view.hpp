#pragma once

#include <scwx/common/color_table.hpp>
#include <scwx/common/products.hpp>
#include <scwx/qt/view/radar_product_view.hpp>

#include <chrono>
#include <memory>
#include <vector>

namespace scwx
{
namespace qt
{
namespace view
{

class Level2ProductView : public RadarProductView
{
   Q_OBJECT

public:
   explicit Level2ProductView(
      common::Level2Product                         product,
      std::shared_ptr<manager::RadarProductManager> radarProductManager);
   ~Level2ProductView();

   std::shared_ptr<common::ColorTable> color_table() const override;
   const std::vector<boost::gil::rgba8_pixel_t>&
                                         color_table_lut() const override;
   std::uint16_t                         color_table_min() const override;
   std::uint16_t                         color_table_max() const override;
   float                                 elevation() const override;
   float                                 range() const override;
   std::chrono::system_clock::time_point sweep_time() const override;
   float                                 unit_scale() const override;
   std::string                           units() const override;
   std::uint16_t                         vcp() const override;
   const std::vector<float>&             vertices() const override;

   void LoadColorTable(std::shared_ptr<common::ColorTable> colorTable) override;
   void SelectElevation(float elevation) override;
   void SelectProduct(const std::string& productName) override;

   common::RadarProductGroup GetRadarProductGroup() const override;
   std::string               GetRadarProductName() const override;
   std::vector<float>        GetElevationCuts() const override;
   std::tuple<const void*, std::size_t, std::size_t>
   GetMomentData() const override;
   std::tuple<const void*, std::size_t, std::size_t>
   GetCfpMomentData() const override;

   std::optional<std::uint16_t>
   GetBinLevel(const common::Coordinate& coordinate) const override;
   std::optional<wsr88d::DataLevelCode>
                        GetDataLevelCode(std::uint16_t level) const override;
   std::optional<float> GetDataValue(std::uint16_t level) const override;

   static std::shared_ptr<Level2ProductView>
   Create(common::Level2Product                         product,
          std::shared_ptr<manager::RadarProductManager> radarProductManager);

protected:
   boost::asio::thread_pool& thread_pool() override;

   void ConnectRadarProductManager() override;
   void DisconnectRadarProductManager() override;
   void UpdateColorTableLut() override;

protected slots:
   void ComputeSweep() override;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace view
} // namespace qt
} // namespace scwx
