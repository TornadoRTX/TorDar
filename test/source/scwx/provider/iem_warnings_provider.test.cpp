#include <scwx/provider/iem_warnings_provider.hpp>

#include <gtest/gtest.h>

namespace scwx
{
namespace provider
{

TEST(IemWarningsProviderTest, ListTextProducts)
{
   using namespace std::chrono;
   using sys_days = time_point<system_clock, days>;

   IemWarningsProvider provider {};

   auto date = sys_days {2023y / March / 25d};

   auto torProducts = provider.ListTextProducts(date, {}, "TOR");

   EXPECT_EQ(torProducts.size(), 35);

   if (torProducts.size() >= 1)
   {
      EXPECT_EQ(torProducts.at(0), "202303250016-KMEG-WFUS54-TORMEG");
   }
   if (torProducts.size() >= 35)
   {
      EXPECT_EQ(torProducts.at(34), "202303252015-KFFC-WFUS52-TORFFC");
   }
}

TEST(IemWarningsProviderTest, LoadTextProducts)
{
   static const std::vector<std::string> productIds {
      "202303250016-KMEG-WFUS54-TORMEG", //
      "202303252015-KFFC-WFUS52-TORFFC"};

   IemWarningsProvider provider {};

   auto textProducts = provider.LoadTextProducts(productIds);

   EXPECT_EQ(textProducts.size(), 2);
}

} // namespace provider
} // namespace scwx
