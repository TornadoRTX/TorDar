#include "scwx/qt/util/check_privilege.hpp"
#include <QtGlobal>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

namespace scwx
{
namespace qt
{
namespace util
{

bool is_high_privilege()
{
#if defined(_WIN32)
   bool            isAdmin = false;
   HANDLE          token   = NULL;
   TOKEN_ELEVATION elevation;
   DWORD           elevationSize = sizeof(TOKEN_ELEVATION);

   if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
   {
      return false;
   }
   if (!GetTokenInformation(
          token, TokenElevation, &elevation, elevationSize, &elevationSize))
   {
      CloseHandle(token);
      return false;
   }
   isAdmin = elevation.TokenIsElevated;
   CloseHandle(token);
   return isAdmin;
#elif defined(Q_OS_UNIX)
   // On UNIX root is always uid 0. On Linux this is enforced by the kernel.
   return geteuid() == 0;
#endif
}

} // namespace util
} // namespace qt
} // namespace scwx
