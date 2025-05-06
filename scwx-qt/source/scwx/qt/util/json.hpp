#pragma once

#include <boost/json/value.hpp>

namespace scwx
{
namespace qt
{
namespace util
{
namespace json
{

boost::json::value ReadJsonQFile(const std::string& path);

} // namespace json
} // namespace util
} // namespace qt
} // namespace scwx
