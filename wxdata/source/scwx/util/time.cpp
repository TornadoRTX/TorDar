// Prevent redefinition of __cpp_lib_format
#if defined(_MSC_VER)
#   include <yvals_core.h>
#endif

// Enable chrono formatters
#ifndef __cpp_lib_format
#   define __cpp_lib_format 202110L
#endif

#include <scwx/network/ntp_client.hpp>
#include <scwx/util/time.hpp>
#include <scwx/util/enum.hpp>
#include <scwx/util/logger.hpp>

#include <sstream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#if (__cpp_lib_chrono < 201907L)
#   include <date/date.h>
#endif

namespace scwx::util::time
{

static const std::string logPrefix_ = "scwx::util::time";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static const std::unordered_map<ClockFormat, std::string> clockFormatName_ {
   {ClockFormat::_12Hour, "12-hour"},
   {ClockFormat::_24Hour, "24-hour"},
   {ClockFormat::Unknown, "?"}};

static std::shared_ptr<network::NtpClient> ntpClient_ {nullptr};

SCWX_GET_ENUM(ClockFormat, GetClockFormat, clockFormatName_)

const std::string& GetClockFormatName(ClockFormat clockFormat)
{
   return clockFormatName_.at(clockFormat);
}

template<typename Clock>
std::chrono::time_point<Clock> now()
{
   if (ntpClient_ == nullptr)
   {
      ntpClient_ = network::NtpClient::Instance();
   }

   if (ntpClient_ != nullptr)
   {
      return Clock::now() + ntpClient_->time_offset();
   }
   else
   {
      return Clock::now();
   }
}

template std::chrono::time_point<std::chrono::system_clock> now();

std::chrono::system_clock::time_point TimePoint(uint32_t modifiedJulianDate,
                                                uint32_t milliseconds)
{
   using namespace std::chrono;
   using sys_days       = time_point<system_clock, days>;
   constexpr auto epoch = sys_days {1969y / December / 31d};

   // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers): literals are used
   return epoch + (modifiedJulianDate * 24h) +
          std::chrono::milliseconds {milliseconds};
}

std::string TimeString(std::chrono::system_clock::time_point time,
                       ClockFormat                           clockFormat,
                       const time_zone*                      timeZone,
                       bool                                  epochValid)
{
   using namespace std::chrono;

#if (__cpp_lib_chrono >= 201907L)
   namespace date = std::chrono;
   namespace df   = std;

   static constexpr std::string_view kFormatString24Hour =
      "{:%Y-%m-%d %H:%M:%S %Z}";
   static constexpr std::string_view kFormatString12Hour =
      "{:%Y-%m-%d %I:%M:%S %p %Z}";
#else
   using namespace date;
   namespace df = date;

#   define kFormatString24Hour "%Y-%m-%d %H:%M:%S %Z"
#   define kFormatString12Hour "%Y-%m-%d %I:%M:%S %p %Z"
#endif

   auto               timeInSeconds = time_point_cast<seconds>(time);
   std::ostringstream os;

   if (epochValid || time.time_since_epoch().count() != 0)
   {
      if (timeZone != nullptr)
      {
         try
         {
            date::zoned_time zt = {timeZone, timeInSeconds};

            if (clockFormat == ClockFormat::_24Hour)
            {
               os << df::format(kFormatString24Hour, zt);
            }
            else
            {
               os << df::format(kFormatString12Hour, zt);
            }
         }
         catch (const std::exception& ex)
         {
            static bool firstException = true;
            if (firstException)
            {
               logger_->warn("Zoned time error: {}", ex.what());
               firstException = false;
            }

            // In the case where there is a time zone error (e.g., timezone
            // database, unicode issue, etc.), fall back to UTC
            timeZone = nullptr;
         }
      }

      if (timeZone == nullptr)
      {
         if (clockFormat == ClockFormat::_24Hour)
         {
            os << df::format(kFormatString24Hour, timeInSeconds);
         }
         else
         {
            os << df::format(kFormatString12Hour, timeInSeconds);
         }
      }
   }

   return os.str();
}

template<typename T>
std::optional<std::chrono::sys_time<T>>
TryParseDateTime(const std::string& dateTimeFormat, const std::string& str)
{
   using namespace std::chrono;

#if (__cpp_lib_chrono < 201907L)
   using namespace date;
#endif

   std::optional<std::chrono::sys_time<T>> value = std::nullopt;
   std::chrono::sys_time<T>                dateTime;
   std::istringstream                      ssDateTime {str};

   ssDateTime >> parse(dateTimeFormat, dateTime);

   if (!ssDateTime.fail())
   {
      value = dateTime;
   }

   return value;
}

template std::optional<std::chrono::sys_time<std::chrono::seconds>>
TryParseDateTime<std::chrono::seconds>(const std::string& dateTimeFormat,
                                       const std::string& str);

} // namespace scwx::util::time
