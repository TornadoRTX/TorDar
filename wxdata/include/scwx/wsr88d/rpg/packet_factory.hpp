#pragma once

#include <scwx/wsr88d/rpg/packet.hpp>

namespace scwx::wsr88d::rpg
{

class PacketFactory
{
public:
   explicit PacketFactory() = delete;
   ~PacketFactory()         = delete;

   PacketFactory(const PacketFactory&)            = delete;
   PacketFactory& operator=(const PacketFactory&) = delete;

   PacketFactory(PacketFactory&&) noexcept            = delete;
   PacketFactory& operator=(PacketFactory&&) noexcept = delete;

   static std::shared_ptr<Packet> Create(std::istream& is);
};

} // namespace scwx::wsr88d::rpg
