#pragma once

#include <cmath>
#include <type_traits>

#include <fmt/format.h>

#include <ovis/utils/json.hpp>
#include <ovis/core/math_operations.hpp>
#include <ovis/core/vector_types.hpp>

namespace ovis {

inline constexpr Vector2 ConstructOrthogonalVectorCW(const Vector2& vector) {
  return {vector.y, -vector.x};
}

inline constexpr Vector2 ConstructOrthogonalVectorCCW(const Vector2& vector) {
  return {-vector.y, vector.x};
}

inline constexpr Vector3 ConstructOrthogonalVector(const Vector3& vector) {
  if (std::abs(vector.x) > std::abs(vector.y)) {
    return Vector3{-vector.z, 0.0f, vector.x} / std::sqrt(vector.x * vector.x + vector.z * vector.z);
  } else {
    return Vector3{0.0f, vector.z, -vector.y} / std::sqrt(vector.y * vector.y + vector.z * vector.z);
  }
}

inline constexpr Vector3 Cross(const Vector3& lhs, const Vector3& rhs) {
  // clang-format off
  return {
    lhs.y * rhs.z - rhs.y * lhs.z,
    lhs.z * rhs.x - rhs.z * lhs.x,
    lhs.x * rhs.y - rhs.x * lhs.y
  };
  // clang-format on
}

template <int VECTOR1_INDEX0, int VECTOR1_INDEX1, int VECTOR2_INDEX0, int VECTOR2_INDEX1>
inline constexpr Vector4 Shuffle(const Vector4& vector1, const Vector4& Vector2) {
  return {
      vector1[VECTOR1_INDEX0],
      vector1[VECTOR1_INDEX1],
      Vector2[VECTOR2_INDEX0],
      Vector2[VECTOR2_INDEX1],
  };
}

}  // namespace ovis
