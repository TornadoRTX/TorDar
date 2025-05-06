#include <scwx/types/iem_types.hpp>

#include <boost/json/value_to.hpp>

namespace scwx::types::iem
{

AfosEntry tag_invoke(boost::json::value_to_tag<AfosEntry>,
                     const boost::json::value& jv)
{
   auto& jo = jv.as_object();

   AfosEntry entry {};

   // Required parameters
   entry.index_     = jo.at("index").as_int64();
   entry.entered_   = jo.at("entered").as_string();
   entry.pil_       = jo.at("pil").as_string();
   entry.productId_ = jo.at("product_id").as_string();
   entry.cccc_      = jo.at("cccc").as_string();
   entry.count_     = jo.at("count").as_int64();
   entry.link_      = jo.at("link").as_string();
   entry.textLink_  = jo.at("text_link").as_string();

   return entry;
}

AfosList tag_invoke(boost::json::value_to_tag<AfosList>,
                    const boost::json::value& jv)
{
   auto& jo = jv.as_object();

   AfosList list {};

   // Required parameters
   list.data_ = boost::json::value_to<std::vector<AfosEntry>>(jo.at("data"));

   return list;
}

BadRequest tag_invoke(boost::json::value_to_tag<BadRequest>,
                      const boost::json::value& jv)
{
   auto& jo = jv.as_object();

   BadRequest badRequest {};

   // Required parameters
   badRequest.detail_ = jo.at("detail").as_string();

   return badRequest;
}

ValidationError::Detail::Context
tag_invoke(boost::json::value_to_tag<ValidationError::Detail::Context>,
           const boost::json::value& jv)
{
   auto& jo = jv.as_object();

   ValidationError::Detail::Context ctx {};

   // Required parameters
   ctx.error_ = jo.at("error").as_string();

   return ctx;
}

ValidationError::Detail
tag_invoke(boost::json::value_to_tag<ValidationError::Detail>,
           const boost::json::value& jv)
{
   auto& jo = jv.as_object();

   ValidationError::Detail detail {};

   // Required parameters
   detail.type_ = jo.at("type").as_string();
   detail.loc_  = boost::json::value_to<
       std::vector<std::variant<std::int64_t, std::string>>>(jo.at("loc"));
   detail.msg_ = jo.at("msg").as_string();

   // Optional parameters
   if (jo.contains("input"))
   {
      detail.input_ = jo.at("input").as_string();
   }

   if (jo.contains("ctx"))
   {
      detail.ctx_ =
         boost::json::value_to<ValidationError::Detail::Context>(jo.at("ctx"));
   }

   return detail;
}

ValidationError tag_invoke(boost::json::value_to_tag<ValidationError>,
                           const boost::json::value& jv)
{
   auto& jo = jv.as_object();

   ValidationError error {};

   // Required parameters
   error.detail_ = boost::json::value_to<std::vector<ValidationError::Detail>>(
      jo.at("detail"));

   return error;
}

} // namespace scwx::types::iem
