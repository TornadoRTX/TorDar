#pragma once

#include <memory>

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

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::provider
