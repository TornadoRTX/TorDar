#pragma once

#include <scwx/common/color_table.hpp>
#include <scwx/common/products.hpp>
#include <scwx/qt/view/radar_product_view.hpp>
#include <scwx/wsr88d/rpg/graphic_product_message.hpp>

#include <memory>
#include <vector>

namespace scwx
{
namespace qt
{
namespace view
{

class Level3ProductView : public RadarProductView
{
   Q_OBJECT

public:
   explicit Level3ProductView(
      const std::string&                            product,
      std::shared_ptr<manager::RadarProductManager> radarProductManager);
   virtual ~Level3ProductView();

   std::shared_ptr<common::ColorTable> color_table() const override;
   const std::vector<boost::gil::rgba8_pixel_t>&
                 color_table_lut() const override;
   std::uint16_t color_table_min() const override;
   std::uint16_t color_table_max() const override;
   float         unit_scale() const override;
   std::string   units() const override;

   void LoadColorTable(std::shared_ptr<common::ColorTable> colorTable) override;

   common::RadarProductGroup GetRadarProductGroup() const override;
   std::string               GetRadarProductName() const override;

   std::optional<wsr88d::DataLevelCode>
                        GetDataLevelCode(std::uint16_t level) const override;
   std::optional<float> GetDataValue(std::uint16_t level) const override;
   bool                 IgnoreUnits() const override;

   void SelectProduct(const std::string& productName) override;

   std::vector<std::pair<std::string, std::string>>
   GetDescriptionFields() const override;

protected:
   std::shared_ptr<wsr88d::rpg::GraphicProductMessage>
        graphic_product_message() const;
   void set_graphic_product_message(
      std::shared_ptr<wsr88d::rpg::GraphicProductMessage> gpm);

   void ConnectRadarProductManager() override;
   void DisconnectRadarProductManager() override;
   void UpdateColorTableLut() override;

   [[nodiscard]] std::uint8_t ComputeEdgeValue() const;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace view
} // namespace qt
} // namespace scwx
