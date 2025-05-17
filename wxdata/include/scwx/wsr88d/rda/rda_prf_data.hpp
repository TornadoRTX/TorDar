#pragma once

#include <scwx/wsr88d/rda/level2_message.hpp>

namespace scwx::wsr88d::rda
{

class RdaPrfData : public Level2Message
{
public:
   explicit RdaPrfData();
   ~RdaPrfData() override;

   RdaPrfData(const RdaPrfData&)            = delete;
   RdaPrfData& operator=(const RdaPrfData&) = delete;

   RdaPrfData(RdaPrfData&&) noexcept;
   RdaPrfData& operator=(RdaPrfData&&) noexcept;

   bool Parse(std::istream& is) override;

   static std::shared_ptr<RdaPrfData> Create(Level2MessageHeader&& header,
                                             std::istream&         is);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::wsr88d::rda
