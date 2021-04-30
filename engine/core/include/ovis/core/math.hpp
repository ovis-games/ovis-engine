#pragma once

#include <span>

#include <ovis/core/vector.hpp>

namespace ovis {

bool IsConvex(std::span<Vector2> vertices);

}  // namespace ovis
