#pragma once

#include <scwx/awips/message.hpp>
#include <scwx/wsr88d/wsr88d_types.hpp>

#include <cstdint>
#include <memory>
#include <optional>

#include <units/angle.h>

namespace scwx::wsr88d::rpg
{

class ProductDescriptionBlockImpl;

class ProductDescriptionBlock : public awips::Message
{
public:
   explicit ProductDescriptionBlock();
   ~ProductDescriptionBlock() override;

   ProductDescriptionBlock(const ProductDescriptionBlock&)            = delete;
   ProductDescriptionBlock& operator=(const ProductDescriptionBlock&) = delete;

   ProductDescriptionBlock(ProductDescriptionBlock&&) noexcept;
   ProductDescriptionBlock& operator=(ProductDescriptionBlock&&) noexcept;

   [[nodiscard]] int16_t  block_divider() const;
   [[nodiscard]] float    latitude_of_radar() const;
   [[nodiscard]] float    longitude_of_radar() const;
   [[nodiscard]] int16_t  height_of_radar() const;
   [[nodiscard]] int16_t  product_code() const;
   [[nodiscard]] uint16_t operational_mode() const;
   [[nodiscard]] uint16_t volume_coverage_pattern() const;
   [[nodiscard]] int16_t  sequence_number() const;
   [[nodiscard]] uint16_t volume_scan_number() const;
   [[nodiscard]] uint16_t volume_scan_date() const;
   [[nodiscard]] uint32_t volume_scan_start_time() const;
   [[nodiscard]] uint16_t generation_date_of_product() const;
   [[nodiscard]] uint32_t generation_time_of_product() const;
   [[nodiscard]] uint16_t elevation_number() const;
   [[nodiscard]] uint16_t data_level_threshold(size_t i) const;
   [[nodiscard]] uint8_t  version() const;
   [[nodiscard]] uint8_t  spot_blank() const;
   [[nodiscard]] uint32_t offset_to_symbology() const;
   [[nodiscard]] uint32_t offset_to_graphic() const;
   [[nodiscard]] uint32_t offset_to_tabular() const;

   [[nodiscard]] float    range() const;
   [[nodiscard]] uint16_t range_raw() const;
   [[nodiscard]] float    x_resolution() const;
   [[nodiscard]] uint16_t x_resolution_raw() const;
   [[nodiscard]] float    y_resolution() const;
   [[nodiscard]] uint16_t y_resolution_raw() const;

   [[nodiscard]] uint16_t threshold() const;
   [[nodiscard]] float    offset() const;
   [[nodiscard]] float    scale() const;
   [[nodiscard]] uint16_t number_of_levels() const;

   [[nodiscard]] std::optional<DataLevelCode>
                                      data_level_code(std::uint8_t level) const;
   [[nodiscard]] std::optional<float> data_value(std::uint8_t level) const;

   [[nodiscard]] std::uint16_t log_start() const;
   [[nodiscard]] float         log_offset() const;
   [[nodiscard]] float         log_scale() const;

   [[nodiscard]] float gr_scale() const;

   [[nodiscard]] std::uint8_t data_mask() const;
   [[nodiscard]] std::uint8_t topped_mask() const;

   [[nodiscard]] units::angle::degrees<double> elevation() const;
   [[nodiscard]] bool                          has_elevation() const;

   [[nodiscard]] bool IsCompressionEnabled() const;
   [[nodiscard]] bool IsDataLevelCoded() const;

   [[nodiscard]] size_t data_size() const override;

   bool Parse(std::istream& is) override;

   static constexpr size_t SIZE = 102u;

private:
   std::unique_ptr<ProductDescriptionBlockImpl> p;
};

} // namespace scwx::wsr88d::rpg
