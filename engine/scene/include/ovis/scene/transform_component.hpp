#pragma once

#include <ovis/math/matrix.hpp>
#include <ovis/math/transform.hpp>
#include <ovis/scene/scene_object_component.hpp>

namespace ovis {

const json TRANSFORM_COMPONENT_SCHEMA = {{"$ref", "base#/$defs/transform"}};

class TransformComponent : public SimpleSceneObjectComponent<Transform, &TRANSFORM_COMPONENT_SCHEMA> {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE(TransformComponent)

 public:
  Matrix3x4 matrix() const { return matrix_; }
  Matrix3x4 inverse_matrix() const { return inverse_matrix_; }

  static void RegisterType(sol::table* module);

 private:
  Matrix3x4 matrix_;
  Matrix3x4 inverse_matrix_;
};
// static_assert(sizeof(TransformComponent) == 4 * 4 * 9 + 8 + 8);
// 9 vec4s: (1 position + 1 rotation + 1 scalint + 2 * 3 matrices)
// + 2 pointer (scene_object + vtable)

}  // namespace ovis
