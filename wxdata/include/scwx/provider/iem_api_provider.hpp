#pragma once

#include <scwx/awips/text_product_file.hpp>

#include <memory>
#include <string>

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

   static std::vector<std::string>
   ListTextProducts(std::chrono::sys_time<std::chrono::days> date,
                    std::optional<std::string_view>          cccc = {},
                    std::optional<std::string_view>          pil  = {});

   static std::vector<std::shared_ptr<awips::TextProductFile>>
   LoadTextProducts(const std::vector<std::string>& textProducts);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::provider
