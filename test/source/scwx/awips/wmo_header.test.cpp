#include <scwx/awips/wmo_header.hpp>

#include <gtest/gtest.h>

namespace scwx::awips
{

static const std::string logPrefix_ = "scwx::awips::wmo_header.test";

static const std::string kWmoHeaderSample_ {
   "887\n"
   "WFUS54 KOUN 280044\n"
   "TOROUN"};

TEST(WmoHeader, WmoFields)
{
   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;
   bool              valid = header.Parse(ss);

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.sequence_number(), "887");
   EXPECT_EQ(header.data_type(), "WF");
   EXPECT_EQ(header.geographic_designator(), "US");
   EXPECT_EQ(header.bulletin_id(), "54");
   EXPECT_EQ(header.icao(), "KOUN");
   EXPECT_EQ(header.date_time(), "280044");
   EXPECT_EQ(header.bbb_indicator(), "");
   EXPECT_EQ(header.product_category(), "TOR");
   EXPECT_EQ(header.product_designator(), "OUN");
   EXPECT_EQ(header.GetDateTime(),
             std::chrono::sys_time<std::chrono::minutes> {});
}

TEST(WmoHeader, DateHintBeforeParse)
{
   using namespace std::chrono;

   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;

   header.SetDateHint(2022y / October);
   bool valid = header.Parse(ss);

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.GetDateTime(),
             sys_days {2022y / October / 28d} + 0h + 44min);
}

TEST(WmoHeader, DateHintAfterParse)
{
   using namespace std::chrono;

   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;

   bool valid = header.Parse(ss);
   header.SetDateHint(2022y / October);

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.GetDateTime(),
             sys_days {2022y / October / 28d} + 0h + 44min);
}

TEST(WmoHeader, EndTimeHintSameMonth)
{
   using namespace std::chrono;

   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;

   bool valid = header.Parse(ss);

   auto endTimeHint = sys_days {2022y / October / 29d} + 0h + 0min + 0s;

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.GetDateTime(endTimeHint),
             sys_days {2022y / October / 28d} + 0h + 44min);
}

TEST(WmoHeader, EndTimeHintPreviousMonth)
{
   using namespace std::chrono;

   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;

   bool valid = header.Parse(ss);

   auto endTimeHint = sys_days {2022y / October / 27d} + 0h + 0min + 0s;

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.GetDateTime(endTimeHint),
             sys_days {2022y / September / 28d} + 0h + 44min);
}

TEST(WmoHeader, EndTimeHintPreviousYear)
{
   using namespace std::chrono;

   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;

   bool valid = header.Parse(ss);

   auto endTimeHint = sys_days {2022y / January / 27d} + 0h + 0min + 0s;

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.GetDateTime(endTimeHint),
             sys_days {2021y / December / 28d} + 0h + 44min);
}

TEST(WmoHeader, EndTimeHintIgnored)
{
   using namespace std::chrono;

   std::stringstream ss {kWmoHeaderSample_};
   WmoHeader         header;

   header.SetDateHint(2022y / October);
   bool valid = header.Parse(ss);

   auto endTimeHint = sys_days {2020y / January / 1d} + 0h + 0min + 0s;

   EXPECT_EQ(valid, true);
   EXPECT_EQ(header.GetDateTime(endTimeHint),
             sys_days {2022y / October / 28d} + 0h + 44min);
}

} // namespace scwx::awips
