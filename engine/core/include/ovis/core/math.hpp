#pragma once

#include <span>

#include <ovis/core/vector.hpp>

namespace ovis {

bool IsConvex(std::span<const Vector2> vertices);
size_t GetInsertPosition(std::span<const Vector2> convex_polygon, Vector2 new_position);

}  // namespace ovis
