#pragma once

#include <scwx/awips/text_product_file.hpp>
#include <scwx/types/iem_types.hpp>
#include <scwx/util/logger.hpp>

#include <memory>
#include <string>
#include <vector>

#include <boost/outcome/result.hpp>
#include <cpr/session.h>
#include <range/v3/range/concepts.hpp>

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4702)
#endif

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
   ListTextProducts(std::chrono::sys_days           date,
                    std::optional<std::string_view> cccc = {},
                    std::optional<std::string_view> pil  = {});

   template<ranges::forward_range DateRange,
            ranges::forward_range CcccRange,
            ranges::forward_range PilRange>
      requires std::same_as<ranges::range_value_t<DateRange>,
                            std::chrono::sys_days> &&
               std::same_as<ranges::range_value_t<CcccRange>,
                            std::string_view> &&
               std::same_as<ranges::range_value_t<PilRange>, std::string_view>
   static boost::outcome_v2::result<std::vector<types::iem::AfosEntry>>
   ListTextProducts(DateRange dates, CcccRange ccccs, PilRange pils);

   template<ranges::forward_range Range>
      requires std::same_as<ranges::range_value_t<Range>, std::string>
   static std::vector<std::shared_ptr<awips::TextProductFile>>
   LoadTextProducts(const Range& textProducts);

private:
   class Impl;
   std::unique_ptr<Impl> p;

   static boost::outcome_v2::result<std::vector<types::iem::AfosEntry>>
   ProcessTextProductLists(std::vector<cpr::AsyncResponse>& asyncResponses);
   static std::vector<std::shared_ptr<awips::TextProductFile>>
   ProcessTextProductFiles(
      std::vector<std::pair<std::string, cpr::AsyncResponse>>& asyncResponses);

   static const std::shared_ptr<spdlog::logger> logger_;

   static const std::string kBaseUrl_;
   static const std::string kListNwsTextProductsEndpoint_;
   static const std::string kNwsTextProductEndpoint_;
};

} // namespace scwx::provider

#include <scwx/provider/iem_api_provider.ipp>
