#pragma once

#include <string>
#include <variant>

#include <boost/json/value.hpp>

namespace scwx::types::iem
{

/**
 * @brief AFOS Entry object
 *
 * <https://mesonet.agron.iastate.edu/api/1/docs#/nws/service_nws_afos_list__fmt__get>
 */
struct AfosEntry
{
   std::int64_t index_ {};
   std::string  entered_ {};
   std::string  pil_ {};
   std::string  productId_ {};
   std::string  cccc_ {};
   std::int64_t count_ {};
   std::string  link_ {};
   std::string  textLink_ {};
};

/**
 * @brief AFOS List object
 *
 * <https://mesonet.agron.iastate.edu/api/1/docs#/nws/service_nws_afos_list__fmt__get>
 */
struct AfosList
{
   std::vector<AfosEntry> data_ {};
};

/**
 * @brief Bad Request (400) object
 */
struct BadRequest
{
   std::string detail_ {};
};

/**
 * @brief Validation Error (422) object
 */
struct ValidationError
{
   struct Detail
   {
      std::string                                          type_ {};
      std::vector<std::variant<std::int64_t, std::string>> loc_ {};
      std::string                                          msg_ {};
      std::string                                          input_ {};
      struct Context
      {
         std::string error_ {};
      } ctx_;
   };

   std::vector<Detail> detail_ {};
};

AfosList        tag_invoke(boost::json::value_to_tag<AfosList>,
                           const boost::json::value& jv);
BadRequest      tag_invoke(boost::json::value_to_tag<BadRequest>,
                           const boost::json::value& jv);
ValidationError tag_invoke(boost::json::value_to_tag<ValidationError>,
                           const boost::json::value& jv);

} // namespace scwx::types::iem
