#pragma once

#include <cstdint>
#include <ovis/math/color_type.hpp>
#include <ovis/math/shared_operations.hpp>

namespace ovis {

constexpr uint32_t ConvertToRGBA8(const Color& color) {
  const uint32_t r = static_cast<uint32_t>(color.r * 255.0f);
  const uint32_t g = static_cast<uint32_t>(color.g * 255.0f);
  const uint32_t b = static_cast<uint32_t>(color.b * 255.0f);
  const uint32_t a = static_cast<uint32_t>(color.a * 255.0f);

  return (r << 24) | (g << 16) | (b << 8) | a;
}

}  // namespace ovis
