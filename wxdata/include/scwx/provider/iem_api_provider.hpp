#pragma once

#include <scwx/awips/text_product_file.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/types/iem_types.hpp>

#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include <boost/outcome/result.hpp>
#include <cpr/cpr.h>

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4702)
#endif

#include <range/v3/view/any_view.hpp>

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

namespace scwx::provider
{

/**
 * @brief Warnings Provider
 */
class IemApiProvider
{
public:
   explicit IemApiProvider();
   ~IemApiProvider();

   IemApiProvider(const IemApiProvider&)            = delete;
   IemApiProvider& operator=(const IemApiProvider&) = delete;

   IemApiProvider(IemApiProvider&&) noexcept;
   IemApiProvider& operator=(IemApiProvider&&) noexcept;

   static boost::outcome_v2::result<std::vector<types::iem::AfosEntry>>
   ListTextProducts(std::chrono::sys_time<std::chrono::days> date,
                    std::optional<std::string_view>          cccc = {},
                    std::optional<std::string_view>          pil  = {});
   static boost::outcome_v2::result<std::vector<types::iem::AfosEntry>>
   ListTextProducts(
      ranges::any_view<std::chrono::sys_time<std::chrono::days>> dates,
      ranges::any_view<std::string_view>                         ccccs = {},
      ranges::any_view<std::string_view>                         pils  = {});

   template<std::ranges::forward_range Range>
      requires std::same_as<std::ranges::range_value_t<Range>, std::string>
   static std::vector<std::shared_ptr<awips::TextProductFile>>
   LoadTextProducts(const Range& textProducts)
   {
      auto parameters = cpr::Parameters {{"nolimit", "true"}};

      std::vector<std::pair<std::string, cpr::AsyncResponse>> asyncResponses {};
      asyncResponses.reserve(textProducts.size());

      const std::string endpointUrl = kBaseUrl_ + kNwsTextProductEndpoint_;

      for (const auto& productId : textProducts)
      {
         asyncResponses.emplace_back(
            productId,
            cpr::GetAsync(cpr::Url {endpointUrl + productId},
                          network::cpr::GetHeader(),
                          parameters));
      }

      return ProcessTextProductResponses(asyncResponses);
   }

private:
   class Impl;
   std::unique_ptr<Impl> p;

   static const std::string kBaseUrl_;
   static const std::string kListNwsTextProductsEndpoint_;
   static const std::string kNwsTextProductEndpoint_;

   static std::vector<std::shared_ptr<awips::TextProductFile>>
   ProcessTextProductResponses(
      std::vector<std::pair<std::string, cpr::AsyncResponse>>& asyncResponses);
};

} // namespace scwx::provider
