#pragma once

#include <ovis/engine/camera.hpp>
#include <ovis/engine/scene_object_component.hpp>
#include <ovis/base/transform_component.hpp>

namespace ovis {

const json CAMERA_COMPONENT_SCHEMA = {{"$ref", "engine#/$defs/camera"}};

class CameraComponent : public SimpleSceneObjectComponent<Camera, &CAMERA_COMPONENT_SCHEMA> {
 public:
  vector3 NormalizedDeviceCoordinatesToViewSpace(vector3 ndc) {
    matrix4 ivverse_projection_matrix = glm::inverse(CalculateProjectionMatrix());
    vector4 view_space = ivverse_projection_matrix * vector4(ndc, 1.0f);
    return view_space / view_space.w;
  }

  vector3 NormalizedDeviceCoordinatesToWorldSpace(vector3 ndc) {
    vector3 world_space_position = NormalizedDeviceCoordinatesToViewSpace(ndc);

    if (scene_object() != nullptr && scene_object()->HasComponent("Transform")) {
      TransformComponent* transform = scene_object()->GetComponent<TransformComponent>("Transform");
      assert(transform != nullptr);

      world_space_position = transform->CalculateGlobalMatrix() * vector4(world_space_position, 1.0f);
    }
    return world_space_position;
  }
};

}  // namespace ovis
