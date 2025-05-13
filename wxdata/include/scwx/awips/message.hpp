#pragma once

#include <array>
#include <cstring>
#include <execution>
#include <istream>
#include <map>
#include <string>

#ifdef _WIN32
#   include <WinSock2.h>
#else
#   include <arpa/inet.h>
#endif

namespace scwx::awips
{

class Message
{
protected:
   explicit Message();

   Message(const Message&)            = delete;
   Message& operator=(const Message&) = delete;

   Message(Message&&) noexcept;
   Message& operator=(Message&&) noexcept;

   virtual bool ValidateMessage(std::istream& is, std::size_t bytesRead) const;

public:
   virtual ~Message();

   virtual std::size_t data_size() const = 0;

   virtual bool Parse(std::istream& is) = 0;

   static void ReadBoolean(std::istream& is, bool& value)
   {
      std::string data(4, ' ');
      is.read(reinterpret_cast<char*>(&data[0]), 4);
      value = (data.at(0) == 'T');
   }

   static void ReadChar(std::istream& is, char& value)
   {
      std::string data(4, ' ');
      is.read(reinterpret_cast<char*>(&data[0]), 4);
      value = data.at(0);
   }

   static float SwapFloat(float f)
   {
      std::uint32_t temp;
      std::memcpy(&temp, &f, sizeof(std::uint32_t));
      temp = ntohl(temp);
      std::memcpy(&f, &temp, sizeof(float));
      return f;
   }

   static double SwapDouble(double d)
   {
      std::uint64_t temp;
      std::memcpy(&temp, &d, sizeof(std::uint64_t));
      temp = ntohll(temp);
      std::memcpy(&d, &temp, sizeof(float));
      return d;
   }

   template<std::size_t _Size>
   static void SwapArray(std::array<float, _Size>& arr,
                         std::size_t               size = _Size)
   {
      std::transform(std::execution::par_unseq,
                     arr.begin(),
                     arr.begin() + size,
                     arr.begin(),
                     [](float f) { return SwapFloat(f); });
   }

   template<std::size_t _Size>
   static void SwapArray(std::array<std::int16_t, _Size>& arr,
                         std::size_t                      size = _Size)
   {
      std::transform(std::execution::par_unseq,
                     arr.begin(),
                     arr.begin() + size,
                     arr.begin(),
                     [](std::int16_t u) { return ntohs(u); });
   }

   template<std::size_t _Size>
   static void SwapArray(std::array<std::uint16_t, _Size>& arr,
                         std::size_t                       size = _Size)
   {
      std::transform(std::execution::par_unseq,
                     arr.begin(),
                     arr.begin() + size,
                     arr.begin(),
                     [](std::uint16_t u) { return ntohs(u); });
   }

   template<std::size_t _Size>
   static void SwapArray(std::array<std::uint32_t, _Size>& arr,
                         std::size_t                       size = _Size)
   {
      std::transform(std::execution::par_unseq,
                     arr.begin(),
                     arr.begin() + size,
                     arr.begin(),
                     [](std::uint32_t u) { return ntohl(u); });
   }

   template<typename T>
   static void SwapMap(std::map<T, float>& m)
   {
      std::for_each(std::execution::par_unseq,
                    m.begin(),
                    m.end(),
                    [](auto& p) { p.second = SwapFloat(p.second); });
   }

   template<typename T>
   static void SwapVector(std::vector<T>& v)
   {
      std::transform(std::execution::par_unseq,
                     v.begin(),
                     v.end(),
                     v.begin(),
                     [](T u)
                     {
                        if constexpr (std::is_same_v<T, std::uint16_t> ||
                                      std::is_same_v<T, std::int16_t>)
                        {
                           return static_cast<T>(ntohs(u));
                        }
                        else if constexpr (std::is_same_v<T, std::uint32_t> ||
                                           std::is_same_v<T, std::int32_t>)
                        {
                           return static_cast<T>(ntohl(u));
                        }
                        else if constexpr (std::is_same_v<T, std::uint64_t> ||
                                           std::is_same_v<T, std::int64_t>)
                        {
                           return static_cast<T>(ntohll(u));
                        }
                        else if constexpr (std::is_same_v<T, float>)
                        {
                           return SwapFloat(u);
                        }
                        else if constexpr (std::is_same_v<T, double>)
                        {
                           return SwapDouble(u);
                        }
                        else
                        {
                           static_assert(std::is_same_v<T, void>,
                                         "Unsupported type for SwapVector");
                        }
                     });
   }

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::awips
