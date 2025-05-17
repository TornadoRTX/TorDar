#pragma once

#include <scwx/wsr88d/rda/level2_message.hpp>

namespace scwx::wsr88d::rda
{

struct Level2MessageInfo
{
   std::shared_ptr<Level2Message> message {nullptr};
   bool                           headerValid {false};
   bool                           messageValid {false};
};

class Level2MessageFactory
{
public:
   explicit Level2MessageFactory() = delete;
   ~Level2MessageFactory()         = delete;

   Level2MessageFactory(const Level2MessageFactory&)            = delete;
   Level2MessageFactory& operator=(const Level2MessageFactory&) = delete;

   Level2MessageFactory(Level2MessageFactory&&) noexcept            = delete;
   Level2MessageFactory& operator=(Level2MessageFactory&&) noexcept = delete;

   struct Context;

   static std::shared_ptr<Context> CreateContext();
   static Level2MessageInfo        Create(std::istream&             is,
                                          std::shared_ptr<Context>& ctx);
};

} // namespace scwx::wsr88d::rda
