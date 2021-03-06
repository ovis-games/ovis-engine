#pragma once

#include <cstdint>

#include <ovis/math/color_type.hpp>
#include <ovis/math/shared_operations.hpp>

namespace ovis {

constexpr uint32_t ConvertToRGBA8(const Color& color) {
  const uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
  const uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
  const uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
  const uint8_t a = static_cast<uint8_t>(color.a * 255.0f);

  // TODO: This breaks for big endian:
  return (r << 0) | (g << 8) | (b << 16) | (a << 24);
}

}  // namespace ovis
