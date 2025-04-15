#include <scwx/provider/iem_api_provider.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/util/json.hpp>
#include <scwx/util/logger.hpp>

#include <boost/json.hpp>
#include <cpr/cpr.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/single.hpp>

#if (__cpp_lib_chrono < 201907L)
#   include <date/date.h>
#endif

namespace scwx::provider
{

static const std::string logPrefix_ = "scwx::provider::iem_api_provider";
static const auto        logger_    = util::Logger::Create(logPrefix_);

static const std::string kBaseUrl_ = "https://mesonet.agron.iastate.edu/api/1";

static const std::string kListNwsTextProductsEndpoint_ = "/nws/afos/list.json";
static const std::string kNwsTextProductEndpoint_      = "/nwstext/";

class IemApiProvider::Impl
{
public:
   explicit Impl()               = default;
   ~Impl()                       = default;
   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;
};

IemApiProvider::IemApiProvider() : p(std::make_unique<Impl>()) {}
IemApiProvider::~IemApiProvider() = default;

IemApiProvider::IemApiProvider(IemApiProvider&&) noexcept            = default;
IemApiProvider& IemApiProvider::operator=(IemApiProvider&&) noexcept = default;

boost::outcome_v2::result<std::vector<types::iem::AfosEntry>>
IemApiProvider::ListTextProducts(std::chrono::sys_time<std::chrono::days> date,
                                 std::optional<std::string_view> optionalCccc,
                                 std::optional<std::string_view> optionalPil)
{
   std::string_view cccc =
      optionalCccc.has_value() ? optionalCccc.value() : std::string_view {};
   std::string_view pil =
      optionalPil.has_value() ? optionalPil.value() : std::string_view {};

   const auto dateArray = std::array {date};
   const auto ccccArray = std::array {cccc};
   const auto pilArray  = std::array {pil};

   return ListTextProducts(dateArray, ccccArray, pilArray);
}

boost::outcome_v2::result<std::vector<types::iem::AfosEntry>>
IemApiProvider::ListTextProducts(
   ranges::any_view<std::chrono::sys_time<std::chrono::days>> dates,
   ranges::any_view<std::string_view>                         ccccs,
   ranges::any_view<std::string_view>                         pils)
{
   using namespace std::chrono;

#if (__cpp_lib_chrono >= 201907L)
   namespace df = std;

   static constexpr std::string_view kDateFormat {"{:%Y-%m-%d}"};
#else
   using namespace date;
   namespace df = date;

#   define kDateFormat "%Y-%m-%d"
#endif

   if (ccccs.begin() == ccccs.end())
   {
      ccccs = ranges::views::single(std::string_view {});
   }

   if (pils.begin() == pils.end())
   {
      pils = ranges::views::single(std::string_view {});
   }

   const auto dv = ranges::to<std::vector>(dates);
   const auto cv = ranges::to<std::vector>(ccccs);
   const auto pv = ranges::to<std::vector>(pils);

   std::vector<cpr::AsyncResponse> responses {};

   for (const auto& [date, cccc, pil] :
        ranges::views::cartesian_product(dv, cv, pv))
   {
      auto parameters =
         cpr::Parameters {{"date", df::format(kDateFormat, date)}};

      // WMO Source Code
      if (!cccc.empty())
      {
         parameters.Add({"cccc", std::string {cccc}});
      }

      // AFOS / AWIPS ID / 3-6 length identifier
      if (!pil.empty())
      {
         parameters.Add({"pil", std::string {pil}});
      }

      responses.emplace_back(
         cpr::GetAsync(cpr::Url {kBaseUrl_ + kListNwsTextProductsEndpoint_},
                       network::cpr::GetHeader(),
                       parameters));
   }

   std::vector<types::iem::AfosEntry> textProducts {};

   for (auto& asyncResponse : responses)
   {
      auto response = asyncResponse.get();

      boost::json::value json = util::json::ReadJsonString(response.text);

      if (response.status_code == cpr::status::HTTP_OK)
      {
         try
         {
            // Get AFOS list from response
            auto entries = boost::json::value_to<types::iem::AfosList>(json);
            textProducts.insert(textProducts.end(),
                                std::make_move_iterator(entries.data_.begin()),
                                std::make_move_iterator(entries.data_.end()));
         }
         catch (const std::exception& ex)
         {
            // Unexpected bad response
            logger_->warn("Error parsing JSON: {}", ex.what());
            return boost::system::errc::make_error_code(
               boost::system::errc::bad_message);
         }
      }
      else if (response.status_code == cpr::status::HTTP_BAD_REQUEST &&
               json != nullptr)
      {
         try
         {
            // Log bad request details
            auto badRequest =
               boost::json::value_to<types::iem::BadRequest>(json);
            logger_->warn("ListTextProducts bad request: {}",
                          badRequest.detail_);
         }
         catch (const std::exception& ex)
         {
            // Unexpected bad response
            logger_->warn("Error parsing bad response: {}", ex.what());
         }

         return boost::system::errc::make_error_code(
            boost::system::errc::invalid_argument);
      }
      else if (response.status_code == cpr::status::HTTP_UNPROCESSABLE_ENTITY &&
               json != nullptr)
      {
         try
         {
            // Log validation error details
            auto error =
               boost::json::value_to<types::iem::ValidationError>(json);
            logger_->warn("ListTextProducts validation error: {}",
                          error.detail_.at(0).msg_);
         }
         catch (const std::exception& ex)
         {
            // Unexpected bad response
            logger_->warn("Error parsing validation error: {}", ex.what());
         }

         return boost::system::errc::make_error_code(
            boost::system::errc::no_message_available);
      }
      else
      {
         logger_->warn("Could not list text products: {}",
                       response.status_line);

         return boost::system::errc::make_error_code(
            boost::system::errc::no_message);
      }
   }

   logger_->trace("Found {} products", textProducts.size());

   return textProducts;
}

std::vector<std::shared_ptr<awips::TextProductFile>>
IemApiProvider::LoadTextProducts(const std::vector<std::string>& textProducts)
{
   auto parameters = cpr::Parameters {{"nolimit", "true"}};

   std::vector<std::pair<const std::string&, cpr::AsyncResponse>>
      asyncResponses {};
   asyncResponses.reserve(textProducts.size());

   const std::string endpointUrl = kBaseUrl_ + kNwsTextProductEndpoint_;

   for (auto& productId : textProducts)
   {
      asyncResponses.emplace_back(
         productId,
         cpr::GetAsync(cpr::Url {endpointUrl + productId},
                       network::cpr::GetHeader(),
                       parameters));
   }

   std::vector<std::shared_ptr<awips::TextProductFile>> textProductFiles;

   for (auto& asyncResponse : asyncResponses)
   {
      auto response = asyncResponse.second.get();

      if (response.status_code == cpr::status::HTTP_OK)
      {
         // Load file
         auto& productId = asyncResponse.first;
         std::shared_ptr<awips::TextProductFile> textProductFile {
            std::make_shared<awips::TextProductFile>()};
         std::istringstream responseBody {response.text};
         if (textProductFile->LoadData(productId, responseBody))
         {
            textProductFiles.push_back(textProductFile);
         }
      }
      else
      {
         logger_->warn("Could not load text product: {} ({})",
                       asyncResponse.first,
                       response.status_line);
      }
   }

   return textProductFiles;
}

} // namespace scwx::provider
