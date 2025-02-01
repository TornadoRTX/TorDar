#pragma once

#include <scwx/awips/text_product_file.hpp>

namespace scwx
{
namespace provider
{

/**
 * @brief Warnings Provider
 */
class WarningsProvider
{
public:
   explicit WarningsProvider(const std::string& baseUrl);
   ~WarningsProvider();

   WarningsProvider(const WarningsProvider&)            = delete;
   WarningsProvider& operator=(const WarningsProvider&) = delete;

   WarningsProvider(WarningsProvider&&) noexcept;
   WarningsProvider& operator=(WarningsProvider&&) noexcept;

   std::vector<std::shared_ptr<awips::TextProductFile>>
   LoadUpdatedFiles(std::chrono::sys_time<std::chrono::hours> newerThan = {});

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace provider
} // namespace scwx
