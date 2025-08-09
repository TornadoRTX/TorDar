#include <scwx/network/ntp_client.hpp>

#include <gtest/gtest.h>

namespace scwx
{
namespace network
{

TEST(NtpClient, Poll)
{
   NtpClient client {};

   client.Open("time.nist.gov", "123");
   //client.Open("pool.ntp.org", "123");
   //client.Open("time.windows.com", "123");
   client.Poll();
}

} // namespace network
} // namespace scwx
