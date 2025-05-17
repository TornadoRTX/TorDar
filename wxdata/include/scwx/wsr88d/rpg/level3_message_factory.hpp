#pragma once

#include <scwx/wsr88d/rpg/level3_message.hpp>

namespace scwx::wsr88d::rpg
{

class Level3MessageFactory
{
public:
   explicit Level3MessageFactory() = delete;
   ~Level3MessageFactory()         = delete;

   Level3MessageFactory(const Level3MessageFactory&)            = delete;
   Level3MessageFactory& operator=(const Level3MessageFactory&) = delete;

   Level3MessageFactory(Level3MessageFactory&&) noexcept            = delete;
   Level3MessageFactory& operator=(Level3MessageFactory&&) noexcept = delete;

   static std::shared_ptr<Level3Message> Create(std::istream& is);
};

} // namespace scwx::wsr88d::rpg
