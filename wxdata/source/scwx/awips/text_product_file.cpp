#include <scwx/awips/text_product_file.hpp>
#include <scwx/util/logger.hpp>

#include <fstream>

#include <re2/re2.h>

namespace scwx
{
namespace awips
{

static const std::string logPrefix_ = "scwx::awips::text_product_file";
static const auto        logger_    = util::Logger::Create(logPrefix_);

class TextProductFileImpl
{
public:
   explicit TextProductFileImpl() : messages_ {} {};
   ~TextProductFileImpl() = default;

   std::vector<std::shared_ptr<TextProductMessage>> messages_;
};

TextProductFile::TextProductFile() : p(std::make_unique<TextProductFileImpl>())
{
}
TextProductFile::~TextProductFile() = default;

TextProductFile::TextProductFile(TextProductFile&&) noexcept = default;
TextProductFile&
TextProductFile::operator=(TextProductFile&&) noexcept = default;

size_t TextProductFile::message_count() const
{
   return p->messages_.size();
}

std::vector<std::shared_ptr<TextProductMessage>>
TextProductFile::messages() const
{
   return p->messages_;
}

std::shared_ptr<TextProductMessage> TextProductFile::message(size_t i) const
{
   return p->messages_[i];
}

bool TextProductFile::LoadFile(const std::string& filename)
{
   logger_->debug("LoadFile: {}", filename);
   bool fileValid = true;

   std::ifstream f(filename, std::ios_base::in | std::ios_base::binary);
   if (!f.good())
   {
      logger_->warn("Could not open file for reading: {}", filename);
      fileValid = false;
   }

   if (fileValid)
   {
      fileValid = LoadData(filename, f);
   }

   return fileValid;
}

bool TextProductFile::LoadData(std::string_view filename, std::istream& is)
{
   static constexpr LazyRE2 kDateTimePattern_ = {
      R"(((?:19|20)\d{2}))"      // Year (YYYY)
      R"((0[1-9]|1[0-2]))"       // Month (MM)
      R"((0[1-9]|[12]\d|3[01]))" // Day (DD)
      R"(_?)"                    // Optional separator (not captured)
      R"(([01]\d|2[0-3]))"       // Hour (HH)
   };

   logger_->trace("Loading Data");

   // Attempt to parse the date from the filename
   std::optional<std::chrono::year_month> yearMonth;
   int                                    year {};
   unsigned int                           month {};

   if (RE2::PartialMatch(filename, *kDateTimePattern_, &year, &month))
   {
      yearMonth = std::chrono::year {year} / std::chrono::month {month};
   }

   while (!is.eof())
   {
      std::shared_ptr<TextProductMessage> message =
         TextProductMessage::Create(is);
      bool duplicate = false;

      if (message != nullptr)
      {
         for (auto m : p->messages_)
         {
            if (*m->wmo_header().get() == *message->wmo_header().get())
            {
               duplicate = true;
               break;
            }
         }

         if (!duplicate)
         {
            if (yearMonth.has_value())
            {
               message->wmo_header()->SetDateHint(yearMonth.value());
            }

            p->messages_.push_back(message);
         }
      }
      else
      {
         break;
      }
   }

   return !p->messages_.empty();
}

} // namespace awips
} // namespace scwx
