#pragma once

#include <memory>

namespace scwx::qt::main
{

bool is_high_privilege();

class PrivilegeChecker
{
public:
   explicit PrivilegeChecker();
   ~PrivilegeChecker();

   PrivilegeChecker(const PrivilegeChecker&)             = delete;
   PrivilegeChecker& operator=(const PrivilegeChecker&)  = delete;
   PrivilegeChecker(const PrivilegeChecker&&)            = delete;
   PrivilegeChecker& operator=(const PrivilegeChecker&&) = delete;

   // returning true means check failed.
   bool pre_settings_check();
   bool post_settings_check();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::main
