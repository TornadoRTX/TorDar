#include <scwx/util/streams.hpp>
#include <scwx/common/characters.hpp>

namespace scwx::util
{

std::istream& getline(std::istream& is, std::string& t)
{
   t.clear();

   std::istream::sentry sentry(is, true);
   std::streambuf*      sb = is.rdbuf();

   while (true)
   {
      int c = sb->sbumpc();
      switch (c)
      {
      case '\n':
         return is;

      case '\r':
         while (sb->sgetc() == '\r')
         {
            sb->sbumpc();
         }
         if (sb->sgetc() == '\n')
         {
            sb->sbumpc();
         }
         return is;

      case common::Characters::ETX:
         sb->sungetc();
         return is;

      case std::streambuf::traits_type::eof():
         if (t.empty())
         {
            is.setstate(std::ios::eofbit);
         }
         return is;

      default:
         t += static_cast<char>(c);
      }
   }
}

} // namespace scwx::util
