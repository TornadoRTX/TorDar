#pragma once

#include <scwx/awips/pvtec.hpp>
#include <scwx/awips/wmo_header.hpp>

namespace scwx
{
namespace qt
{
namespace types
{

struct TextEventKey
{
   TextEventKey() : TextEventKey(awips::PVtec {}) {}
   TextEventKey(const awips::PVtec& pvtec, std::chrono::year yearHint = {}) :
       officeId_ {pvtec.office_id()},
       phenomenon_ {pvtec.phenomenon()},
       significance_ {pvtec.significance()},
       etn_ {pvtec.event_tracking_number()}
   {
      using namespace std::chrono_literals;

      std::chrono::year_month_day ymd =
         std::chrono::floor<std::chrono::days>(pvtec.event_begin());
      if (ymd.year() > 1970y)
      {
         // Prefer the year from the event begin
         year_ = ymd.year();
      }
      else if (yearHint > 1970y)
      {
         // Otherwise, use the year hint
         year_ = yearHint;
      }
      else
      {
         // If there was no year hint, use the event end
         ymd   = std::chrono::floor<std::chrono::days>(pvtec.event_end());
         year_ = ymd.year();
      }
   }

   std::string ToFullString() const;
   std::string ToString() const;
   bool        operator==(const TextEventKey& o) const;

   std::string         officeId_;
   awips::Phenomenon   phenomenon_;
   awips::Significance significance_;
   std::int16_t        etn_;
   std::chrono::year   year_;
};

template<class Key>
struct TextEventHash;

template<>
struct TextEventHash<TextEventKey>
{
   size_t operator()(const TextEventKey& x) const;
};

} // namespace types
} // namespace qt
} // namespace scwx
