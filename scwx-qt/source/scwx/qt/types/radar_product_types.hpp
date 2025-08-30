#pragma once

#include <cstdint>

namespace scwx::qt::types
{

enum class RadarProductLoadStatus : std::uint8_t
{
   ProductLoaded,
   ListingProducts,
   LoadingProduct,
   ProductNotAvailable
};

}
