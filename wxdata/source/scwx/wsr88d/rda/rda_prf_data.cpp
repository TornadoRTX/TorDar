#include <scwx/wsr88d/rda/rda_prf_data.hpp>
#include <scwx/util/logger.hpp>

namespace scwx::wsr88d::rda
{

static const std::string logPrefix_ = "scwx::wsr88d::rda::rda_prf_data";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

struct RdaPrfWaveformData
{
   std::uint16_t              waveformType_ {0};
   std::uint16_t              prfCount_ {0};
   std::vector<std::uint32_t> prfValues_ {};
};

class RdaPrfData::Impl
{
public:
   explicit Impl() = default;
   ~Impl()         = default;

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   std::uint16_t                   numberOfWaveforms_ {0};
   std::vector<RdaPrfWaveformData> waveformData_ {};
};

RdaPrfData::RdaPrfData() : p(std::make_unique<Impl>()) {}
RdaPrfData::~RdaPrfData() = default;

RdaPrfData::RdaPrfData(RdaPrfData&&) noexcept            = default;
RdaPrfData& RdaPrfData::operator=(RdaPrfData&&) noexcept = default;

bool RdaPrfData::Parse(std::istream& is)
{
   logger_->trace("Parsing RDA PRF Data (Message Type 32)");

   bool        messageValid = true;
   std::size_t bytesRead    = 0;

   const std::streampos isBegin = is.tellg();

   is.read(reinterpret_cast<char*>(&p->numberOfWaveforms_), 2); // 1
   is.seekg(2, std::ios_base::cur);                             // 2

   bytesRead += 4;

   p->numberOfWaveforms_ = ntohs(p->numberOfWaveforms_);

   // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers): Readability
   if (p->numberOfWaveforms_ < 1 || p->numberOfWaveforms_ > 5)
   {
      logger_->warn("Invalid number of waveforms: {}", p->numberOfWaveforms_);
      p->numberOfWaveforms_ = 0;
      messageValid          = false;
   }

   p->waveformData_.resize(p->numberOfWaveforms_);

   for (std::uint16_t i = 0; i < p->numberOfWaveforms_; ++i)
   {
      auto& w = p->waveformData_[i];

      is.read(reinterpret_cast<char*>(&w.waveformType_), 2); // P1
      is.read(reinterpret_cast<char*>(&w.prfCount_), 2);     // P2

      w.waveformType_ = ntohs(w.waveformType_);
      w.prfCount_     = ntohs(w.prfCount_);

      bytesRead += 4;

      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers): Readability
      if (w.prfCount_ > 255)
      {
         logger_->warn("Invalid PRF count: {} (waveform {})", w.prfCount_, i);
         w.prfCount_  = 0;
         messageValid = false;
         break;
      }

      w.prfValues_.resize(w.prfCount_);

      for (std::uint16_t j = 0; j < w.prfCount_; ++j)
      {
         is.read(reinterpret_cast<char*>(&w.prfValues_[j]), 4);
      }

      bytesRead += static_cast<std::size_t>(w.prfCount_) * 4;

      SwapVector(w.prfValues_);
   }

   is.seekg(isBegin, std::ios_base::beg);
   if (!ValidateMessage(is, bytesRead))
   {
      messageValid = false;
   }

   return messageValid;
}

std::shared_ptr<RdaPrfData> RdaPrfData::Create(Level2MessageHeader&& header,
                                               std::istream&         is)
{
   std::shared_ptr<RdaPrfData> message = std::make_shared<RdaPrfData>();
   message->set_header(std::move(header));

   if (!message->Parse(is))
   {
      message.reset();
   }

   return message;
}

} // namespace scwx::wsr88d::rda
