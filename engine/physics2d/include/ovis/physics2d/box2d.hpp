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

template <typename It>
inline std::vector<Vector2> FromBox2DVec2s(It begin, It end) {
  static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(*begin)>>, b2Vec2>);
  std::vector<Vector2> vectors;
  vectors.reserve(end - begin);
  std::transform(begin, end, std::back_inserter(vectors), FromBox2DVec2);
  return vectors;
}

template <typename It>
inline std::vector<b2Vec2> ToBox2DVec2s(It begin, It end) {
  static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(*begin)>>, Vector2>);
  std::vector<b2Vec2> vectors;
  vectors.reserve(end - begin);
  std::transform(begin, end, std::back_inserter(vectors), ToBox2DVec2);
  return vectors;
}

}  // namespace ovis
