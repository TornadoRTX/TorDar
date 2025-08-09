#pragma once

#include <memory>
#include <string_view>

namespace scwx::network
{

/**
 * @brief NTP Client
 */
class NtpClient
{
public:
   explicit NtpClient();
   ~NtpClient();

   NtpClient(const NtpClient&)            = delete;
   NtpClient& operator=(const NtpClient&) = delete;

   NtpClient(NtpClient&&) noexcept;
   NtpClient& operator=(NtpClient&&) noexcept;

   void Open(std::string_view host, std::string_view service);
   void Poll();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::network
