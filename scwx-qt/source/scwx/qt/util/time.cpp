#include <scwx/qt/util/time.hpp>

namespace scwx
{
namespace qt
{
namespace util
{

std::chrono::sys_days SysDays(const QDate& date)
{
   using namespace std::chrono;
   using sys_days             = time_point<system_clock, days>;
   constexpr auto julianEpoch = sys_days {-4713y / November / 24d};

   return std::chrono::sys_days(std::chrono::days(date.toJulianDay()) +
                                julianEpoch);
}

local_days LocalDays(const QDate& date)
{
#if (__cpp_lib_chrono >= 201907L)
   using namespace std::chrono;
#else
   using namespace date;
#endif
   auto yearMonthDay =
      year_month_day(year(date.year()), month(date.month()), day(date.day()));
   return local_days(yearMonthDay);
}

} // namespace util
} // namespace qt
} // namespace scwx
