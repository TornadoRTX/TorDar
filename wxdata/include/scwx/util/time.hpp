#pragma once

#include <scwx/util/iterator.hpp>

#include <chrono>
#include <optional>
#include <string>

#if (__cpp_lib_chrono < 201907L)
#   include <date/tz.h>
#endif

namespace scwx::util::time
{

#if (__cpp_lib_chrono >= 201907L)
typedef std::chrono::time_zone time_zone;
#else
typedef date::time_zone time_zone;
#endif

enum class ClockFormat
{
   _12Hour,
   _24Hour,
   Unknown
};
typedef scwx::util::
   Iterator<ClockFormat, ClockFormat::_12Hour, ClockFormat::_24Hour>
      ClockFormatIterator;

ClockFormat        GetClockFormat(const std::string& name);
const std::string& GetClockFormatName(ClockFormat clockFormat);

template<typename Clock = std::chrono::system_clock>
std::chrono::time_point<Clock> now();

std::chrono::system_clock::time_point TimePoint(uint32_t modifiedJulianDate,
                                                uint32_t milliseconds);

std::string TimeString(std::chrono::system_clock::time_point time,
                       ClockFormat      clockFormat = ClockFormat::_24Hour,
                       const time_zone* timeZone    = nullptr,
                       bool             epochValid  = true);

template<typename T>
std::optional<std::chrono::sys_time<T>>
TryParseDateTime(const std::string& dateTimeFormat, const std::string& str);

} // namespace scwx::util::time

namespace scwx::util
{
// Add types and functions to scwx::util for compatibility
using time::ClockFormat;
using time::ClockFormatIterator;
using time::GetClockFormat;
using time::GetClockFormatName;
using time::time_zone;
using time::TimePoint;
using time::TimeString;
using time::TryParseDateTime;
} // namespace scwx::util
