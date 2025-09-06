#pragma once

#include <string>

namespace scwx
{
namespace qt
{
namespace types
{

enum class AnimationState
{
   Play,
   Pause
};

enum class MapTime
{
   Live,
   Archive
};

enum class NoUpdateReason
{
   NoChange,
   NotLoaded,
   NotAvailable,
   InvalidProduct,
   InvalidData
};

std::string GetMapTimeName(MapTime mapTime);

} // namespace types
} // namespace qt
} // namespace scwx
