#pragma once

#include <scwx/wsr88d/rpg/packet.hpp>

#include <cstdint>
#include <memory>

namespace scwx::wsr88d::rpg
{

class DigitalRasterDataArrayPacket : public Packet
{
public:
   explicit DigitalRasterDataArrayPacket();
   ~DigitalRasterDataArrayPacket();

   DigitalRasterDataArrayPacket(const DigitalRasterDataArrayPacket&) = delete;
   DigitalRasterDataArrayPacket&
   operator=(const DigitalRasterDataArrayPacket&) = delete;

   DigitalRasterDataArrayPacket(DigitalRasterDataArrayPacket&&) noexcept;
   DigitalRasterDataArrayPacket&
   operator=(DigitalRasterDataArrayPacket&&) noexcept;

   [[nodiscard]] std::uint16_t packet_code() const override;
   [[nodiscard]] std::uint16_t i_coordinate_start() const;
   [[nodiscard]] std::uint16_t j_coordinate_start() const;
   [[nodiscard]] std::uint16_t i_scale_factor() const;
   [[nodiscard]] std::uint16_t j_scale_factor() const;
   [[nodiscard]] std::uint16_t number_of_cells() const;
   [[nodiscard]] std::uint16_t number_of_rows() const;

   [[nodiscard]] std::uint16_t number_of_bytes_in_row(std::uint16_t r) const;
   [[nodiscard]] const std::vector<std::uint8_t>& level(std::uint16_t r) const;

   [[nodiscard]] std::size_t data_size() const override;

   bool Parse(std::istream& is) override;

   static std::shared_ptr<DigitalRasterDataArrayPacket>
   Create(std::istream& is);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::wsr88d::rpg
