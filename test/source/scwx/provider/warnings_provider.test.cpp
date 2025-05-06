#include <scwx/provider/warnings_provider.hpp>

#include <gtest/gtest.h>

namespace scwx
{
namespace provider
{

static const std::string& kDefaultUrl {"https://warnings.allisonhouse.com"};
static const std::string& kAlternateUrl {"https://warnings.cod.edu"};

class WarningsProviderTest : public testing::TestWithParam<std::string>
{
};

TEST_P(WarningsProviderTest, LoadUpdatedFiles)
{
   WarningsProvider provider(GetParam());

   const std::chrono::sys_time<std::chrono::hours> now =
      std::chrono::floor<std::chrono::hours>(std::chrono::system_clock::now());
   const std::chrono::sys_time<std::chrono::hours> startTime =
      now - std::chrono::days {3};

   auto updatedFiles = provider.LoadUpdatedFiles(startTime);

   // No objects, skip test
   if (updatedFiles.empty())
   {
      GTEST_SKIP();
   }

   EXPECT_GT(updatedFiles.size(), 0);

   auto updatedFiles2 = provider.LoadUpdatedFiles();
}

INSTANTIATE_TEST_SUITE_P(WarningsProvider,
                         WarningsProviderTest,
                         testing::Values(kDefaultUrl, kAlternateUrl));

} // namespace provider
} // namespace scwx
