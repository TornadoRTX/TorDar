#pragma once

#include <array>
#include <bit>
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
      if constexpr (std::endian::native == std::endian::little)
      {
         // Variable is initialized by memcpy
         // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
         std::uint32_t temp;
         std::memcpy(&temp, &f, sizeof(std::uint32_t));
         temp = ntohl(temp);
         std::memcpy(&f, &temp, sizeof(float));
      }
      return f;
   }

   static double SwapDouble(double d)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         // Variable is initialized by memcpy
         // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
         std::uint64_t temp;
         std::memcpy(&temp, &d, sizeof(std::uint64_t));
         temp = Swap64(temp);
         std::memcpy(&d, &temp, sizeof(float));
      }
      return d;
   }

   static std::uint64_t Swap64(std::uint64_t value)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
         std::uint32_t high = ntohl(static_cast<std::uint32_t>(value >> 32));
         std::uint32_t low =
            ntohl(static_cast<std::uint32_t>(value & 0xFFFFFFFFULL));
         return (static_cast<std::uint64_t>(low) << 32) | high;
         // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
      }
      else
      {
         return value;
      }
   }

   template<std::size_t kSize>
   static void SwapArray(std::array<float, kSize>& arr,
                         std::size_t               size = kSize)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         std::transform(std::execution::par_unseq,
                        arr.begin(),
                        arr.begin() + size,
                        arr.begin(),
                        [](float f) { return SwapFloat(f); });
      }
   }

   template<std::size_t kSize>
   static void SwapArray(std::array<std::int16_t, kSize>& arr,
                         std::size_t                      size = kSize)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         std::transform(std::execution::par_unseq,
                        arr.begin(),
                        arr.begin() + size,
                        arr.begin(),
                        [](std::int16_t u) { return ntohs(u); });
      }
   }

   template<std::size_t kSize>
   static void SwapArray(std::array<std::uint16_t, kSize>& arr,
                         std::size_t                       size = kSize)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         std::transform(std::execution::par_unseq,
                        arr.begin(),
                        arr.begin() + size,
                        arr.begin(),
                        [](std::uint16_t u) { return ntohs(u); });
      }
   }

   template<std::size_t kSize>
   static void SwapArray(std::array<std::uint32_t, kSize>& arr,
                         std::size_t                       size = kSize)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         std::transform(std::execution::par_unseq,
                        arr.begin(),
                        arr.begin() + size,
                        arr.begin(),
                        [](std::uint32_t u) { return ntohl(u); });
      }
   }

   template<typename T>
   static void SwapMap(std::map<T, float>& m)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         std::for_each(std::execution::par_unseq,
                       m.begin(),
                       m.end(),
                       [](auto& p) { p.second = SwapFloat(p.second); });
      }
   }

   template<typename T>
   static void SwapVector(std::vector<T>& v)
   {
      if constexpr (std::endian::native == std::endian::little)
      {
         std::transform(
            std::execution::par_unseq,
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
                  return static_cast<T>(Swap64(u));
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
   }

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::awips
