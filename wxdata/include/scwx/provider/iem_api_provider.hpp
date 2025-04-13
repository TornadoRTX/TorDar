#pragma once

#include <scwx/awips/text_product_file.hpp>

#include <memory>
#include <string>
#include <vector>

#include <boost/outcome/result.hpp>

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

   static boost::outcome_v2::result<std::vector<std::string>>
   ListTextProducts(std::chrono::sys_time<std::chrono::days> date,
                    std::optional<std::string_view>          cccc = {},
                    std::optional<std::string_view>          pil  = {});
   static boost::outcome_v2::result<std::vector<std::string>>
   ListTextProducts(std::vector<std::chrono::sys_time<std::chrono::days>> dates,
                    std::vector<std::string_view> ccccs = {},
                    std::vector<std::string_view> pils  = {});

   static std::vector<std::shared_ptr<awips::TextProductFile>>
   LoadTextProducts(const std::vector<std::string>& textProducts);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::provider
