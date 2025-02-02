#include <scwx/provider/iem_api_provider.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/types/iem_types.hpp>
#include <scwx/util/json.hpp>
#include <scwx/util/logger.hpp>

#include <boost/json.hpp>
#include <cpr/cpr.h>

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

IemApiProvider::IemApiProvider(IemApiProvider&&) noexcept =
   default;
IemApiProvider&
IemApiProvider::operator=(IemApiProvider&&) noexcept = default;

std::vector<std::string> IemApiProvider::ListTextProducts(
   std::chrono::sys_time<std::chrono::days> date,
   std::optional<std::string_view>          cccc,
   std::optional<std::string_view>          pil)
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

   auto parameters = cpr::Parameters {{"date", df::format(kDateFormat, date)}};

   // WMO Source Code
   if (cccc.has_value())
   {
      parameters.Add({"cccc", std::string {cccc.value()}});
   }

   // AFOS / AWIPS ID / 3-6 length identifier
   if (pil.has_value())
   {
      parameters.Add({"pil", std::string {pil.value()}});
   }

   auto response =
      cpr::Get(cpr::Url {kBaseUrl_ + kListNwsTextProductsEndpoint_},
               network::cpr::GetHeader(),
               parameters);
   boost::json::value json = util::json::ReadJsonString(response.text);

   std::vector<std::string> textProducts {};

   if (response.status_code == cpr::status::HTTP_OK)
   {
      try
      {
         // Get AFOS list from response
         auto entries = boost::json::value_to<types::iem::AfosList>(json);

         for (auto& entry : entries.data_)
         {
            textProducts.push_back(entry.productId_);
         }

         logger_->trace("Found {} products", entries.data_.size());
      }
      catch (const std::exception& ex)
      {
         // Unexpected bad response
         logger_->warn("Error parsing JSON: {}", ex.what());
      }
   }
   else if (response.status_code == cpr::status::HTTP_BAD_REQUEST &&
            json != nullptr)
   {
      try
      {
         // Log bad request details
         auto badRequest = boost::json::value_to<types::iem::BadRequest>(json);
         logger_->warn("ListTextProducts bad request: {}", badRequest.detail_);
      }
      catch (const std::exception& ex)
      {
         // Unexpected bad response
         logger_->warn("Error parsing bad response: {}", ex.what());
      }
   }
   else if (response.status_code == cpr::status::HTTP_UNPROCESSABLE_ENTITY &&
            json != nullptr)
   {
      try
      {
         // Log validation error details
         auto error = boost::json::value_to<types::iem::ValidationError>(json);
         logger_->warn("ListTextProducts validation error: {}",
                       error.detail_.at(0).msg_);
      }
      catch (const std::exception& ex)
      {
         // Unexpected bad response
         logger_->warn("Error parsing validation error: {}", ex.what());
      }
   }
   else
   {
      logger_->warn("Could not list text products: {}", response.status_line);
   }

   return textProducts;
}

std::vector<std::shared_ptr<awips::TextProductFile>>
IemApiProvider::LoadTextProducts(
   const std::vector<std::string>& textProducts)
{
   auto parameters = cpr::Parameters {{"nolimit", "true"}};

   std::vector<std::pair<std::string_view, cpr::AsyncResponse>>
      asyncResponses {};
   asyncResponses.reserve(textProducts.size());
   
   const std::string endpointUrl = kBaseUrl_ + kNwsTextProductEndpoint_;

   for (auto& productId : textProducts)
   {
      asyncResponses.emplace_back(
         productId,
         cpr::GetAsync(
            cpr::Url {endpointUrl + productId},
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
         std::shared_ptr<awips::TextProductFile> textProductFile {
            std::make_shared<awips::TextProductFile>()};
         std::istringstream responseBody {response.text};
         if (textProductFile->LoadData(responseBody))
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
