#pragma once

#include <glm/geometric.hpp>
#include <ovis/math/basic_types.hpp>

namespace ovis {

struct Triangle {
  Triangle() = default;
  Triangle(const vector3& v0, const vector3& v1, const vector3& v2) : vertices{v0, v1, v2} {}

  vector3 vertices[3];
};

// The vertices are assumed to be counter clockwise.
// The result is not normalized!
// \see CalculateNormalizedNormal
inline vector3 CalculateNormal(Triangle triangle) {
  return glm::cross(triangle.vertices[1] - triangle.vertices[0], triangle.vertices[2] - triangle.vertices[0]);
}

inline vector3 CalculateNormalizedNormal(Triangle triangle) {
  return glm::normalize(CalculateNormal(triangle));
}

}  // namespace ovis