#pragma once

#include <algorithm>
#include <cmath>
#include <type_traits>

#include <fmt/format.h>

#include <ovis/utils/json.hpp>
#include <ovis/core/math_constants.hpp>
#include <ovis/core/quaternion_type.hpp>
#include <ovis/core/math_operations.hpp>
#include <ovis/core/vector.hpp>

namespace ovis {

inline constexpr Quaternion Quaternion::Identity() {
  return {1.0f, 0.0f, 0.0f, 0.0f};
}

inline Quaternion Quaternion::FromAxisAndAngle(const Vector3& axis, float angle) {
  const float half_angle = 0.5f * angle;
  const float sin_half_angle = std::sin(half_angle);
  const float cos_half_angle = std::cos(half_angle);

  return {
      cos_half_angle,
      sin_half_angle * axis.x,
      sin_half_angle * axis.y,
      sin_half_angle * axis.z,
  };
}

inline Quaternion Quaternion::FromEulerAngles(float yaw, float pitch, float roll) {
  const float cy = std::cos(yaw * 0.5);
  const float sy = std::sin(yaw * 0.5);
  const float cp = std::cos(pitch * 0.5);
  const float sp = std::sin(pitch * 0.5);
  const float cr = std::cos(roll * 0.5);
  const float sr = std::sin(roll * 0.5);

  // TODO: switch stuff around
  return {
      cr * cp * cy + sr * sp * sy,
      cr * sp * cy + sr * cp * sy,
      cr * cp * sy - sr * sp * cy,
      sr * cp * cy - cr * sp * sy,
  };
}

inline Quaternion Quaternion::FromRotationMatrix(const Matrix3& r) {
  // Implementation from here: https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
  return {
    std::sqrt(std::max(0.0f, 1.0f + r[0][0] + r[1][1] + r[2][2])) * 0.5f,
    std::copysign(std::sqrt(std::max(0.0f, 1.0f + r[0][0] - r[1][1] - r[2][2])) * 0.5f, r[2][1] - r[1][2]),
    std::copysign(std::sqrt(std::max(0.0f, 1.0f - r[0][0] + r[1][1] - r[2][2])) * 0.5f, r[0][2] - r[2][0]),
    std::copysign(std::sqrt(std::max(0.0f, 1.0f - r[0][0] - r[1][1] + r[2][2])) * 0.5f, r[1][0] - r[0][1])
  };
}

inline Quaternion Conjugate(const Quaternion& quaternion) {
  return {quaternion.w, -quaternion.x, -quaternion.y, -quaternion.z};
}

inline Quaternion Invert(const Quaternion& quaternion) {
  return Conjugate(quaternion) / Dot(quaternion, quaternion);
}

inline float ExtractYaw(const Quaternion& q) {
  return std::asin(std::clamp(-2.0f * (q.x * q.z - q.w * q.y), -1.0f, 1.0f));
}

inline float ExtractPitch(const Quaternion& q) {
  return std::atan2(2.0f * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
}

inline float ExtractRoll(const Quaternion& q) {
  return std::atan2(2.0f * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);
}

inline Vector3 ExtractEulerAngles(const Quaternion& quaternion) {
  return {
      ExtractPitch(quaternion),
      ExtractYaw(quaternion),
      ExtractRoll(quaternion),
  };
}

inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) {
  return {
    // clang-format off
    lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
    lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
    lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
    lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x
    // clang-format on
  };
}

inline Vector3 operator*(const Quaternion& quaternion, const Vector3& vector) {
  const Vector3 quaternion_vector = {quaternion.x, quaternion.y, quaternion.z};
  const Vector3 uv = Cross(quaternion_vector, vector);
  const Vector3 uuv = Cross(quaternion_vector, uv);

  return vector + ((uv * quaternion.w) + uuv) * 2.0f;
}

inline Vector3 operator*(const Vector3& vector, const Quaternion& quaternion) {
  return Invert(quaternion) * vector;
}

inline Quaternion operator*=(Quaternion& lhs, const Quaternion& rhs) {
  for (int i = 0; i < 4; ++i) {
    lhs[i] *= rhs[i];
  }
  return lhs;
}

}  // namespace ovis
