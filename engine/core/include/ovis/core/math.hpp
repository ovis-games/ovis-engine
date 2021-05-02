#pragma once

#include <span>

#include <ovis/core/vector.hpp>

namespace ovis {

bool IsConvex(std::span<const Vector2> vertices);
size_t GetLineStripInsertPosition(std::span<const Vector2> strip, Vector2 new_position);
size_t GetLineLoopInsertPosition(std::span<const Vector2> loop, Vector2 new_position);

}  // namespace ovis
