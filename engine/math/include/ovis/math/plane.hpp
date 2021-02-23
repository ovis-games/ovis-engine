#pragma once

#include <glm/vec3.hpp>

namespace ovis {

struct Plane {
  vector3 axis;
  float distance;
};
static_assert(sizeof(Plane) == 16, "");

}  // namespace ovis