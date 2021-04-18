#pragma once

#include <box2d/b2_math.h>

#include <ovis/core/vector.hpp>

namespace ovis {

inline Vector2 FromBox2DVec2(b2Vec2 v) {
  return {v.x, v.y};
}

inline b2Vec2 ToBox2DVec2(Vector2 v) {
  return {v.x, v.y};
}

}  // namespace ovis
