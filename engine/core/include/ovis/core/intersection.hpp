#pragma once

#include <optional>

#include <ovis/core/vector.hpp>

namespace ovis {

constexpr float EPSILON = 0.00001f;

template <typename VectorType>
struct Ray {
  static_assert(is_vector<VectorType>::value, "VectorType must be Vector2, Vector3 or Vector4");

  VectorType origin;
  VectorType direction;
};

using Ray2D = Ray<Vector2>;
using Ray3D = Ray<Vector3>;

struct Plane3D {
  Vector4 normal_distance;

  inline constexpr Vector3 normal() const { return normal_distance; }
  inline constexpr Vector3 origin() const { return normal() * normal_distance.w; }

  static inline constexpr Plane3D FromPointAndNormal(Vector3 point, Vector3 normal) {
    const Vector3 normalized_normal = Normalize(normal);
    return {Vector4::FromVector3(normalized_normal, Dot(point, normalized_normal))};
  }

  static inline constexpr Plane3D FromPointAndSpanningVectors(Vector3 point, Vector3 v, Vector3 w) {
    return FromPointAndNormal(point, Cross(v, w));
  }
};
static_assert(sizeof(Plane3D) == sizeof(float) * 4);

inline constexpr std::optional<Vector3> ComputeRayPlaneIntersection(Ray3D ray, Plane3D plane) {
  const Vector3 plane_normal = plane.normal();

  const float denominator = Dot(ray.direction, plane_normal);
  if (std::abs(denominator) < EPSILON) {
    // Ray is close to parallel to the plane
    return {};
  }

  const float t = Dot(plane.origin() - ray.origin, plane_normal) / denominator;
  return ray.origin + t * ray.direction;
}

}  // namespace ovis
