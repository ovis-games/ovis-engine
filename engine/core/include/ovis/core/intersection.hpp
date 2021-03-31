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

template <typename VectorType>
struct AxisAlignedBoundingBox {
  static_assert(is_vector<VectorType>::value, "VectorType must be Vector2, Vector3 or Vector4");

  VectorType center;
  VectorType half_extend;

  static inline constexpr AxisAlignedBoundingBox<VectorType> FromCenterAndExtend(VectorType center, VectorType extend) {
    return {center, 0.5f * extend};
  }

  static inline constexpr AxisAlignedBoundingBox<VectorType> Empty() {
    return {VectorType::Zero(), VectorType::Zero()};
  }
};
using AxisAlignedBoundingBox2D = AxisAlignedBoundingBox<Vector2>;
using AxisAlignedBoundingBox3D = AxisAlignedBoundingBox<Vector3>;

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

struct RayAABBIntersection {
  float t_ray_enter;
  float t_ray_exit;
};

template <typename VectorType>
inline constexpr std::optional<RayAABBIntersection> ComputeRayAABBIntersection(
    Ray<VectorType> ray, AxisAlignedBoundingBox<VectorType> aabb) {
  const VectorType inverse_direction = 1.0f / ray.direction;
  const VectorType aabb_min = aabb.center - aabb.half_extend;
  const VectorType aabb_max = aabb.center + aabb.half_extend;

  const VectorType t0 = (aabb_min - ray.origin) * inverse_direction;
  const VectorType t1 = (aabb_max - ray.origin) * inverse_direction;

  const VectorType t_smaller = min(t0, t1);
  const VectorType t_bigger = max(t0, t1);

  const float t_min = max(0.0f, MaxComponent(t_smaller));
  const float t_max = MinComponent(t_bigger);

  return t_min <= t_max ? std::optional<RayAABBIntersection>(RayAABBIntersection{t_min, t_max})
                        : std::optional<RayAABBIntersection>{};
}

template <typename VectorType>
inline constexpr std::optional<RayAABBIntersection> ComputeRayAABBIntersection(Ray<VectorType> ray,
                                                                      AxisAlignedBoundingBox<VectorType> aabb,
                                                                      float t_min, float t_max) {
  const VectorType inverse_direction = 1.0f / ray.direction;
  const VectorType aabb_min = aabb.center - aabb.half_extend;
  const VectorType aabb_max = aabb.center + aabb.half_extend;

  const VectorType t0 = (aabb_min - ray.origin) * inverse_direction;
  const VectorType t1 = (aabb_max - ray.origin) * inverse_direction;

  const VectorType t_smaller = min(t0, t1);
  const VectorType t_bigger = max(t0, t1);

  t_min = max(t_min, MaxComponent(t_smaller));
  t_max = min(t_max, MinComponent(t_bigger));

  return t_min <= t_max ? std::optional<RayAABBIntersection>(RayAABBIntersection{t_min, t_max})
                        : std::optional<RayAABBIntersection>{};
}

}  // namespace ovis
