#pragma once

#include <ovis/math/transform.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

const json TRANSFORM_COMPONENT_SCHEMA = {{"$ref", "base#/$defs/transform"}};

class TransformComponent : public SimpleSceneObjectComponent<Transform, &TRANSFORM_COMPONENT_SCHEMA> {
 public:
  matrix4 CalculateGlobalMatrix() const { return CalculateMatrix(); }
  matrix4 CalculateGlobalInverseMatrix() const { return CalculateInverseMatrix(); }
};

}  // namespace ovis
