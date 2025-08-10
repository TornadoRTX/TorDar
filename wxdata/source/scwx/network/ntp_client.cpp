#include <scwx/network/ntp_client.hpp>
#include <scwx/types/ntp_types.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/threads.hpp>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>
#include <fmt/chrono.h>

namespace scwx::network
{

static const std::string logPrefix_ = "scwx::network::ntp_client";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static constexpr std::size_t kReceiveBufferSize_ {48u};

class NtpTimestamp
{
public:
   // NTP epoch: January 1, 1900
   // Unix epoch: January 1, 1970
   // Difference = 70 years = 2,208,988,800 seconds
   static constexpr std::uint32_t kNtpToUnixOffset_ = 2208988800UL;

   // NTP fractional part represents 1/2^32 of a second
   static constexpr std::uint64_t kFractionalMultiplier_ = 0x100000000ULL;

   static constexpr std::uint64_t _1e9 = 1000000000ULL;

   std::uint32_t seconds_ {0};
   std::uint32_t fraction_ {0};

   explicit NtpTimestamp() = default;
   explicit NtpTimestamp(std::uint32_t seconds, std::uint32_t fraction) :
       seconds_ {seconds}, fraction_ {fraction}
   {
   }
   ~NtpTimestamp() = default;

   NtpTimestamp(const NtpTimestamp&)            = default;
   NtpTimestamp& operator=(const NtpTimestamp&) = default;
   NtpTimestamp(NtpTimestamp&&)                 = default;
   NtpTimestamp& operator=(NtpTimestamp&&)      = default;

   template<typename Clock = std::chrono::system_clock>
   std::chrono::time_point<Clock> ToTimePoint() const
   {
      // Convert NTP seconds to Unix seconds
      // Don't cast to a larger type to account for rollover, and this should
      // work until 2106
      const std::uint32_t unixSeconds = seconds_ - kNtpToUnixOffset_;

      // Convert NTP fraction to nanoseconds
      const auto nanoseconds =
         static_cast<std::uint64_t>(fraction_) * _1e9 / kFractionalMultiplier_;

      return std::chrono::time_point<Clock>(
         std::chrono::duration_cast<typename Clock::duration>(
            std::chrono::seconds {unixSeconds} +
            std::chrono::nanoseconds {nanoseconds}));
   }

   template<typename Clock = std::chrono::system_clock>
   static NtpTimestamp FromTimePoint(std::chrono::time_point<Clock> timePoint)
   {
      // Convert to duration since Unix epoch
      const auto unixDuration = timePoint.time_since_epoch();

      // Extract seconds and nanoseconds
      const auto unixSeconds =
         std::chrono::duration_cast<std::chrono::seconds>(unixDuration);
      const auto nanoseconds =
         std::chrono::duration_cast<std::chrono::nanoseconds>(unixDuration -
                                                              unixSeconds);

      // Convert Unix seconds to NTP seconds
      const auto ntpSeconds =
         static_cast<std::uint32_t>(unixSeconds.count() + kNtpToUnixOffset_);

      // Convert nanoseconds to NTP fractional seconds
      const auto ntpFraction = static_cast<std::uint32_t>(
         nanoseconds.count() * kFractionalMultiplier_ / _1e9);

      return NtpTimestamp(ntpSeconds, ntpFraction);
   }
};

class NtpClient::Impl
{
public:
   explicit Impl();
   ~Impl();
   Impl(const Impl&)            = delete;
   Impl& operator=(const Impl&) = delete;
   Impl(Impl&&)                 = delete;
   Impl& operator=(Impl&&)      = delete;

   void Open(std::string_view host, std::string_view service);
   void Poll();
   void ReceivePacket(std::size_t length);

   boost::asio::thread_pool threadPool_ {2u};

   bool enabled_;

   types::ntp::NtpPacket transmitPacket_ {};

   boost::asio::ip::udp::socket                  socket_;
   std::optional<boost::asio::ip::udp::endpoint> serverEndpoint_ {};
   std::array<std::uint8_t, kReceiveBufferSize_> receiveBuffer_ {};

   std::chrono::system_clock::duration timeOffset_ {};

   std::vector<std::string> serverList_ {"time.nist.gov",
                                         "time.cloudflare.com",
                                         "ntp.pool.org",
                                         "time.aws.com",
                                         "time.windows.com",
                                         "time.apple.com"};
};

NtpClient::NtpClient() : p(std::make_unique<Impl>()) {}
NtpClient::~NtpClient() = default;

NtpClient::NtpClient(NtpClient&&) noexcept            = default;
NtpClient& NtpClient::operator=(NtpClient&&) noexcept = default;

void NtpClient::Open(std::string_view host, std::string_view service)
{
   p->Open(host, service);
}

void NtpClient::Poll()
{
   p->Poll();
}

NtpClient::Impl::Impl() : socket_ {threadPool_}
{
   using namespace std::chrono_literals;

   const auto now =
      std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());

   // The NTP timestamp will overflow in 2036. Overflow is handled in such a way
   // that should work until 2106. Additional handling for subsequent eras is
   // required.
   static constexpr auto kMaxYear_ = 2106y;

   enabled_ = now < kMaxYear_ / 1 / 1;

   transmitPacket_.fields.vn   = 3; // Version
   transmitPacket_.fields.mode = 3; // Client (3)
}

NtpClient::Impl::~Impl()
{
   threadPool_.join();
}

void NtpClient::Impl::Open(std::string_view host, std::string_view service)
{
   boost::asio::ip::udp::resolver resolver(threadPool_);
   boost::system::error_code      ec;

   auto results = resolver.resolve(host, service, ec);
   if (ec.value() == boost::system::errc::success && !results.empty())
   {
      logger_->info("Using NTP server: {}", host);
      serverEndpoint_ = *results.begin();
      socket_.open(serverEndpoint_->protocol());
   }
   else
   {
      serverEndpoint_ = std::nullopt;
      logger_->warn("Could not resolve host {}: {}", host, ec.message());
   }
}

void NtpClient::Impl::Poll()
{
   using namespace std::chrono_literals;

   static constexpr auto kTimeout_ = 15s;

   try
   {
      const auto originTimestamp =
         NtpTimestamp::FromTimePoint(std::chrono::system_clock::now());
      transmitPacket_.txTm_s = ntohl(originTimestamp.seconds_);
      transmitPacket_.txTm_f = ntohl(originTimestamp.fraction_);

      std::size_t transmitPacketSize = sizeof(transmitPacket_);
      // Send NTP request
      socket_.send_to(boost::asio::buffer(&transmitPacket_, transmitPacketSize),
                      *serverEndpoint_);

      // Receive NTP response
      auto future =
         socket_.async_receive_from(boost::asio::buffer(receiveBuffer_),
                                    *serverEndpoint_,
                                    boost::asio::use_future);
      std::size_t bytesReceived = 0;

      switch (future.wait_for(kTimeout_))
      {
      case std::future_status::ready:
         bytesReceived = future.get();
         ReceivePacket(bytesReceived);
         break;

      case std::future_status::timeout:
      case std::future_status::deferred:
         logger_->warn("Timeout waiting for NTP response");
         socket_.cancel();
         break;
      }
   }
   catch (const std::exception& ex)
   {
      logger_->error("Error polling: {}", ex.what());
   }
}

void NtpClient::Impl::ReceivePacket(std::size_t length)
{
   if (length >= sizeof(types::ntp::NtpPacket))
   {
      const auto destinationTime = std::chrono::system_clock::now();

      const auto packet = types::ntp::NtpPacket::Parse(receiveBuffer_);

      if (packet.stratum == 0)
      {
         const std::uint32_t refId = ntohl(packet.refId);
         const std::string   kod =
            std::string(reinterpret_cast<const char*>(&refId), 4);

         logger_->warn("KoD packet received: {}", kod);

         if (kod == "DENY" || kod == "RSTR")
         {
            // TODO
            // The client MUST demobilize any associations to that server and
            // stop sending packets to that server
         }
         else if (kod == "RATE")
         {
            // TODO
            // The client MUST immediately reduce its polling interval to that
            // server and continue to reduce it each time it receives a RATE
            // kiss code
         }
      }
      else
      {
         const auto originTimestamp =
            NtpTimestamp(packet.origTm_s, packet.origTm_f);
         const auto receiveTimestamp =
            NtpTimestamp(packet.rxTm_s, packet.rxTm_f);
         const auto transmitTimestamp =
            NtpTimestamp(packet.txTm_s, packet.txTm_f);

         const auto originTime   = originTimestamp.ToTimePoint();
         const auto receiveTime  = receiveTimestamp.ToTimePoint();
         const auto transmitTime = transmitTimestamp.ToTimePoint();

         const auto& t0 = originTime;
         const auto& t1 = receiveTime;
         const auto& t2 = transmitTime;
         const auto& t3 = destinationTime;

         // Update time offset
         timeOffset_ = ((t1 - t0) + (t2 - t3)) / 2;

         logger_->debug("Time offset updated: {:%jd %T}", timeOffset_);
      }
   }
   else
   {
      logger_->warn("Received too few bytes: {}", length);
   }
}

} // namespace scwx::network
