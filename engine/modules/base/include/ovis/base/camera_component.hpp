#pragma once

#include <ovis/base/transform_component.hpp>

#include <ovis/math/matrix.hpp>
#include <ovis/engine/camera.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

const json CAMERA_COMPONENT_SCHEMA = {{"$ref", "engine#/$defs/camera"}};

class CameraComponent : public SimpleSceneObjectComponent<Camera, &CAMERA_COMPONENT_SCHEMA> {
 public:
  Vector3 NormalizedDeviceCoordinatesToViewSpace(Vector3 ndc) {
    Matrix4 inverse_projection_matrix = Invert(CalculateProjectionMatrix());
    return TransformPosition(inverse_projection_matrix, ndc);
  }

  Vector3 NormalizedDeviceCoordinatesToWorldSpace(Vector3 ndc) {
    Vector3 world_space_position = NormalizedDeviceCoordinatesToViewSpace(ndc);

    if (scene_object() != nullptr && scene_object()->HasComponent("Transform")) {
      TransformComponent* transform = scene_object()->GetComponent<TransformComponent>("Transform");
      assert(transform != nullptr);

      world_space_position = TransformPosition(transform->CalculateGlobalMatrix(), world_space_position);
    }
    return world_space_position;
  }
};

}  // namespace ovis
