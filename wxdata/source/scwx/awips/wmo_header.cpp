#include <scwx/awips/wmo_header.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/streams.hpp>

#include <istream>
#include <sstream>
#include <string>

#ifdef _WIN32
#   include <WinSock2.h>
#else
#   include <arpa/inet.h>
#endif

namespace scwx::awips
{

static const std::string logPrefix_ = "scwx::awips::wmo_header";
static const auto        logger_    = util::Logger::Create(logPrefix_);

static constexpr std::size_t kWmoHeaderMinLineLength_    = 18;
static constexpr std::size_t kWmoIdentifierLengthMin_    = 5;
static constexpr std::size_t kWmoIdentifierLengthMax_    = 6;
static constexpr std::size_t kIcaoLength_                = 4;
static constexpr std::size_t kDateTimeLength_            = 6;
static constexpr std::size_t kAwipsIdentifierLineLength_ = 6;

class WmoHeaderImpl
{
public:
   explicit WmoHeaderImpl() :
       sequenceNumber_ {},
       dataType_ {},
       geographicDesignator_ {},
       bulletinId_ {},
       icao_ {},
       dateTime_ {},
       bbbIndicator_ {},
       productCategory_ {},
       productDesignator_ {}
   {
   }
   ~WmoHeaderImpl() = default;

   WmoHeaderImpl(const WmoHeaderImpl&)             = delete;
   WmoHeaderImpl& operator=(const WmoHeaderImpl&)  = delete;
   WmoHeaderImpl(const WmoHeaderImpl&&)            = delete;
   WmoHeaderImpl& operator=(const WmoHeaderImpl&&) = delete;

   void CalculateAbsoluteDateTime();
   bool ParseDateTime(unsigned int&  dayOfMonth,
                      unsigned long& hour,
                      unsigned long& minute);

   bool operator==(const WmoHeaderImpl& o) const;

   std::string sequenceNumber_ {};
   std::string dataType_ {};
   std::string geographicDesignator_ {};
   std::string bulletinId_ {};
   std::string icao_ {};
   std::string dateTime_ {};
   std::string bbbIndicator_ {};
   std::string productCategory_ {};
   std::string productDesignator_ {};

   std::optional<std::chrono::year_month> dateHint_ {};
   std::optional<std::chrono::sys_time<std::chrono::minutes>>
      absoluteDateTime_ {};
};

WmoHeader::WmoHeader() : p(std::make_unique<WmoHeaderImpl>()) {}
WmoHeader::~WmoHeader() = default;

WmoHeader::WmoHeader(WmoHeader&&) noexcept            = default;
WmoHeader& WmoHeader::operator=(WmoHeader&&) noexcept = default;

bool WmoHeader::operator==(const WmoHeader& o) const
{
   return (*p.get() == *o.p.get());
}

bool WmoHeaderImpl::operator==(const WmoHeaderImpl& o) const
{
   return (sequenceNumber_ == o.sequenceNumber_ &&             //
           dataType_ == o.dataType_ &&                         //
           geographicDesignator_ == o.geographicDesignator_ && //
           bulletinId_ == o.bulletinId_ &&                     //
           icao_ == o.icao_ &&                                 //
           dateTime_ == o.dateTime_ &&                         //
           bbbIndicator_ == o.bbbIndicator_ &&                 //
           productCategory_ == o.productCategory_ &&           //
           productDesignator_ == o.productDesignator_);
}

std::string WmoHeader::sequence_number() const
{
   return p->sequenceNumber_;
}

std::string WmoHeader::data_type() const
{
   return p->dataType_;
}

std::string WmoHeader::geographic_designator() const
{
   return p->geographicDesignator_;
}

std::string WmoHeader::bulletin_id() const
{
   return p->bulletinId_;
}

std::string WmoHeader::icao() const
{
   return p->icao_;
}

std::string WmoHeader::date_time() const
{
   return p->dateTime_;
}

std::string WmoHeader::bbb_indicator() const
{
   return p->bbbIndicator_;
}

std::string WmoHeader::product_category() const
{
   return p->productCategory_;
}

std::string WmoHeader::product_designator() const
{
   return p->productDesignator_;
}

std::chrono::sys_time<std::chrono::minutes> WmoHeader::GetDateTime(
   std::optional<std::chrono::system_clock::time_point> endTimeHint)
{
   std::chrono::sys_time<std::chrono::minutes> wmoDateTime {};

   if (p->absoluteDateTime_.has_value())
   {
      wmoDateTime = p->absoluteDateTime_.value();
   }
   else if (endTimeHint.has_value())
   {
      bool          dateTimeValid = false;
      unsigned int  dayOfMonth    = 0;
      unsigned long hour          = 0;
      unsigned long minute        = 0;

      dateTimeValid = p->ParseDateTime(dayOfMonth, hour, minute);

      if (dateTimeValid)
      {
         using namespace std::chrono;

         auto           endDays = floor<days>(endTimeHint.value());
         year_month_day endDate {endDays};

         // Combine end date year and month with WMO date time
         wmoDateTime =
            sys_days {endDate.year() / endDate.month() / day {dayOfMonth}} +
            hours {hour} + minutes {minute};

         // If the begin date is after the end date, assume the start time
         // was the previous month (give a 1 day grace period for expiring
         // events in the past)
         // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
         if (wmoDateTime > endTimeHint.value() + 24h)
         {
            // If the current end month is January
            if (endDate.month() == January)
            {
               // The begin month must be December of last year
               wmoDateTime =
                  sys_days {
                     year {static_cast<int>((endDate.year() - 1y).count())} /
                     December / day {dayOfMonth}} +
                  hours {hour} + minutes {minute};
            }
            else
            {
               // Back up one month
               wmoDateTime =
                  sys_days {endDate.year() /
                            month {static_cast<unsigned int>(
                               (endDate.month() - month {1}).count())} /
                            day {dayOfMonth}} +
                  hours {hour} + minutes {minute};
            }
         }
      }
   }

   return wmoDateTime;
}

bool WmoHeader::Parse(std::istream& is)
{
   bool headerValid = true;

   std::string sohLine;
   std::string sequenceLine;
   std::string wmoLine;
   std::string awipsLine;

   if (is.peek() == 0x01)
   {
      util::getline(is, sohLine);
      util::getline(is, sequenceLine);
      util::getline(is, wmoLine);
   }
   else
   {
      // The next line could be the WMO line or the sequence line
      util::getline(is, wmoLine);
      if (wmoLine.length() < kWmoHeaderMinLineLength_)
      {
         // This is likely the sequence line instead
         sequenceLine.swap(wmoLine);
         util::getline(is, wmoLine);
      }
   }

   util::getline(is, awipsLine);

   if (is.eof())
   {
      logger_->trace("Reached end of file");
      headerValid = false;
   }
   else
   {
      // Remove delimiters from the end of the line
      while (sequenceLine.ends_with(' '))
      {
         sequenceLine.erase(sequenceLine.length() - 1);
      }
   }

   // Transmission Header:
   // [SOH]
   // nnn

   if (headerValid && !sequenceLine.empty())
   {
      p->sequenceNumber_ = sequenceLine;
   }

   // WMO Abbreviated Heading Line:
   // T1T2A1A2ii CCCC YYGGgg (BBB)

   if (headerValid)
   {
      std::string              token;
      std::istringstream       wmoTokens(wmoLine);
      std::vector<std::string> wmoTokenList;

      while (wmoTokens >> token)
      {
         wmoTokenList.push_back(token);
      }

      if (wmoTokenList.size() < 3 || wmoTokenList.size() > 4)
      {
         logger_->warn("Invalid number of WMO tokens");
         headerValid = false;
      }
      else if (wmoTokenList[0].size() < kWmoIdentifierLengthMin_ ||
               wmoTokenList[0].size() > kWmoIdentifierLengthMax_)
      {
         logger_->warn("WMO identifier malformed");
         headerValid = false;
      }
      else if (wmoTokenList[1].size() != kIcaoLength_)
      {
         logger_->warn("ICAO malformed");
         headerValid = false;
      }
      else if (wmoTokenList[2].size() != kDateTimeLength_)
      {
         logger_->warn("Date/time malformed");
         headerValid = false;
      }
      else if (wmoTokenList.size() == 4 && wmoTokenList[3].size() != 3)
      {
         // BBB indicator is optional
         logger_->warn("BBB indicator malformed");
         headerValid = false;
      }
      else
      {
         p->dataType_             = wmoTokenList[0].substr(0, 2);
         p->geographicDesignator_ = wmoTokenList[0].substr(2, 2);
         p->bulletinId_ = wmoTokenList[0].substr(4, wmoTokenList[0].size() - 4);
         p->icao_       = wmoTokenList[1];
         p->dateTime_   = wmoTokenList[2];

         p->CalculateAbsoluteDateTime();

         if (wmoTokenList.size() == 4)
         {
            p->bbbIndicator_ = wmoTokenList[3];
         }
         else
         {
            p->bbbIndicator_ = "";
         }
      }
   }

   // AWIPS Identifer Line:
   // NNNxxx

   if (headerValid)
   {
      if (awipsLine.size() != kAwipsIdentifierLineLength_)
      {
         logger_->warn("AWIPS Identifier Line bad size");
         headerValid = false;
      }
      else
      {
         p->productCategory_   = awipsLine.substr(0, 3);
         p->productDesignator_ = awipsLine.substr(3, 3);
      }
   }

   return headerValid;
}

void WmoHeader::SetDateHint(std::chrono::year_month dateHint)
{
   p->dateHint_ = dateHint;
   p->CalculateAbsoluteDateTime();
}

bool WmoHeaderImpl::ParseDateTime(unsigned int&  dayOfMonth,
                                  unsigned long& hour,
                                  unsigned long& minute)
{
   bool dateTimeValid = false;

   try
   {
      // WMO date time is in the format DDHHMM
      dayOfMonth =
         static_cast<unsigned int>(std::stoul(dateTime_.substr(0, 2)));
      hour          = std::stoul(dateTime_.substr(2, 2));
      minute        = std::stoul(dateTime_.substr(4, 2));
      dateTimeValid = true;
   }
   catch (const std::exception&)
   {
      logger_->warn("Malformed WMO date/time: {}", dateTime_);
   }

   return dateTimeValid;
}

void WmoHeaderImpl::CalculateAbsoluteDateTime()
{
   bool dateTimeValid = false;

   if (dateHint_.has_value() && !dateTime_.empty())
   {
      unsigned int  dayOfMonth = 0;
      unsigned long hour       = 0;
      unsigned long minute     = 0;

      dateTimeValid = ParseDateTime(dayOfMonth, hour, minute);

      if (dateTimeValid)
      {
         using namespace std::chrono;
         absoluteDateTime_ = sys_days {dateHint_->year() / dateHint_->month() /
                                       day {dayOfMonth}} +
                             hours {hour} + minutes {minute};
      }
   }

   if (!dateTimeValid)
   {
      absoluteDateTime_.reset();
   }
}

} // namespace scwx::awips
