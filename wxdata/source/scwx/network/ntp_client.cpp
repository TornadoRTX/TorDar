#include <scwx/network/ntp_client.hpp>
#include <scwx/types/ntp_types.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/threads.hpp>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>

namespace scwx::network
{

static const std::string logPrefix_ = "scwx::network::ntp_client";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static constexpr std::size_t kReceiveBufferSize_ {48u};

class NtpClient::Impl
{
public:
   explicit Impl();
   ~Impl();
   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   void Open(std::string_view host, std::string_view service);
   void Poll();
   void ReceivePacket(std::size_t length);

   boost::asio::thread_pool threadPool_ {2u};

   types::ntp::NtpPacket transmitPacket_ {};

   boost::asio::ip::udp::socket                  socket_;
   std::optional<boost::asio::ip::udp::endpoint> serverEndpoint_ {};
   std::array<std::uint8_t, kReceiveBufferSize_> receiveBuffer_ {};

   std::vector<std::string> serverList_ {
      "time.nist.gov", "ntp.pool.org", "time.windows.com"};
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
      auto packet = types::ntp::NtpPacket::Parse(receiveBuffer_);
      (void) packet;
   }
   else
   {
      logger_->warn("Received too few bytes: {}", length);
   }
}

} // namespace scwx::network
