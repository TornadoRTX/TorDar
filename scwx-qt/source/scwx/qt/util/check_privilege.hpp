#pragma once

#include <memory>

namespace scwx::qt::util
{

bool is_high_privilege();

class PrivilegeChecker
{
public:
   explicit PrivilegeChecker();
   ~PrivilegeChecker();

   // returning true means check failed.
   bool first_check();
   bool second_check();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::util
