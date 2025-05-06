#include <scwx/provider/iem_api_provider.ipp>

#include <gtest/gtest.h>

namespace scwx::provider
{

TEST(IemApiProviderTest, ListTextProducts)
{
   using namespace std::chrono;
   using sys_days = time_point<system_clock, days>;

   auto date = sys_days {2023y / March / 25d};

   auto torProducts = IemApiProvider::ListTextProducts(date, {}, "TOR");

   ASSERT_EQ(torProducts.has_value(), true);
   EXPECT_EQ(torProducts.value().size(), 35);

   if (torProducts.value().size() >= 1)
   {
      EXPECT_EQ(torProducts.value().at(0).productId_,
                "202303250016-KMEG-WFUS54-TORMEG");
   }
   if (torProducts.value().size() >= 35)
   {
      EXPECT_EQ(torProducts.value().at(34).productId_,
                "202303252015-KFFC-WFUS52-TORFFC");
   }
}

TEST(IemApiProviderTest, LoadTextProducts)
{
   static const std::vector<std::string> productIds {
      "202303250016-KMEG-WFUS54-TORMEG",
      "202303252015-KFFC-WFUS52-TORFFC",
      "202303311942-KLZK-WWUS54-SVSLZK"};

   auto textProducts = IemApiProvider::LoadTextProducts(productIds);

   EXPECT_EQ(textProducts.size(), 3);

   if (textProducts.size() >= 1)
   {
      EXPECT_EQ(textProducts.at(0)->message_count(), 1);
   }
   if (textProducts.size() >= 2)
   {
      EXPECT_EQ(textProducts.at(1)->message_count(), 1);
   }
   if (textProducts.size() >= 3)
   {
      EXPECT_EQ(textProducts.at(2)->message_count(), 2);
   }
}

} // namespace scwx::provider
