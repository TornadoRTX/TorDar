#pragma once

#include <memory>
#include <string>

namespace scwx::provider
{

/**
 * @brief Warnings Provider
 */
class IemWarningsProvider
{
public:
   explicit IemWarningsProvider();
   ~IemWarningsProvider();

   IemWarningsProvider(const IemWarningsProvider&)            = delete;
   IemWarningsProvider& operator=(const IemWarningsProvider&) = delete;

   IemWarningsProvider(IemWarningsProvider&&) noexcept;
   IemWarningsProvider& operator=(IemWarningsProvider&&) noexcept;

   std::vector<std::string>
   ListTextProducts(std::chrono::sys_time<std::chrono::days> date,
                    std::optional<std::string_view>          cccc = {},
                    std::optional<std::string_view>          pil  = {});

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::provider
