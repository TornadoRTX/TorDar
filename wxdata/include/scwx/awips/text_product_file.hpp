#pragma once

#include <scwx/awips/text_product_message.hpp>

#include <memory>
#include <string>

namespace scwx::awips
{

class TextProductFileImpl;

class TextProductFile
{
public:
   explicit TextProductFile();
   ~TextProductFile();

   TextProductFile(const TextProductFile&)            = delete;
   TextProductFile& operator=(const TextProductFile&) = delete;

   TextProductFile(TextProductFile&&) noexcept;
   TextProductFile& operator=(TextProductFile&&) noexcept;

   [[nodiscard]] std::size_t message_count() const;
   [[nodiscard]] std::vector<std::shared_ptr<TextProductMessage>>
                                                     messages() const;
   [[nodiscard]] std::shared_ptr<TextProductMessage> message(size_t i) const;

   bool LoadFile(const std::string& filename);
   bool LoadData(std::string_view filename, std::istream& is);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::awips
