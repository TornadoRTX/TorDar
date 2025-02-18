#pragma once

#include <chrono>
#include <memory>
#include <string>

namespace scwx
{
namespace awips
{

class WmoHeaderImpl;

/**
 * @brief The WMO Header is defined in WMO Manual No. 386, with additional codes
 * defined in WMO Codes Manual 306.  The NWS summarizes the relevant
 * information.
 *
 * <https://www.roc.noaa.gov/WSR88D/Level_III/Level3Info.aspx>
 * <https://www.weather.gov/tg/head>
 * <https://www.weather.gov/tg/headef>
 * <https://www.weather.gov/tg/bbb>
 * <https://www.weather.gov/tg/awips>
 */
class WmoHeader
{
public:
   explicit WmoHeader();
   ~WmoHeader();

   WmoHeader(const WmoHeader&)            = delete;
   WmoHeader& operator=(const WmoHeader&) = delete;

   WmoHeader(WmoHeader&&) noexcept;
   WmoHeader& operator=(WmoHeader&&) noexcept;

   bool operator==(const WmoHeader& o) const;

   std::string sequence_number() const;
   std::string data_type() const;
   std::string geographic_designator() const;
   std::string bulletin_id() const;
   std::string icao() const;
   std::string date_time() const;
   std::string bbb_indicator() const;
   std::string product_category() const;
   std::string product_designator() const;

   /**
    * @brief Get the WMO date/time
    *
    * Gets the WMO date/time. Uses the optional date hint provided via
    * SetDateHint(std::chrono::year_month). If the date hint has not been
    * provided, the endTimeHint parameter is required.
    *
    * @param [in] endTimeHint The optional end time bounds to provide. This is
    * ignored if a date hint has been provided to determine an absolute date.
    */
   std::chrono::sys_time<std::chrono::minutes> GetDateTime(
      std::optional<std::chrono::system_clock::time_point> endTimeHint =
         std::nullopt);

   /**
    * @brief Parse a WMO header
    *
    * @param [in] is The input stream to parse
    */
   bool Parse(std::istream& is);

   /**
    * @brief Provide a date hint for the WMO parser
    *
    * The WMO header contains a date/time in the format DDMMSS. The year and
    * month must be derived using another source. The date hint provides the
    * additional context required to determine the absolute product time.
    *
    * This function will update any absolute date/time already calculated, or
    * affect the calculation of a subsequent absolute date/time.
    *
    * @param [in] dateHint The date hint to provide the WMO header parser
    */
   void SetDateHint(std::chrono::year_month dateHint);

private:
   std::unique_ptr<WmoHeaderImpl> p;
};

} // namespace awips
} // namespace scwx
