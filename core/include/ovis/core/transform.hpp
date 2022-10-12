#pragma once

#include "ovis/core/matrix.hpp"
#include "ovis/core/quaternion.hpp"
#include "ovis/core/simple_scene_controller.hpp"
#include "ovis/core/vector.hpp"
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

struct Transform {
  Vector3 position = Vector3::Zero();
  Vector3 scale = Vector3::One();
  Quaternion rotation = Quaternion::Identity();

  OVIS_VM_DECLARE_TYPE_BINDING();
};

void to_json(json& data, const Transform& transform);
void from_json(const json& data, Transform& transform);

struct LocalTransformMatrices {
  Matrix3x4 parent_to_local = Matrix3x4::IdentityTransformation();
  Matrix3x4 local_to_parent = Matrix3x4::IdentityTransformation();

  OVIS_VM_DECLARE_TYPE_BINDING();
};

void to_json(json& data, const Transform& transform);
void from_json(const json& data, Transform& transform);

void ComputeLocalTransformMatrices(const Transform&, LocalTransformMatrices* local_transform_matrices);
class LocalTransformMatricesController : public SimpleSceneController<&ComputeLocalTransformMatrices> {
 public:
  LocalTransformMatricesController() : SimpleSceneController("ComputeLocalTransformMatrices") {}
};

struct GlobalTransformMatrices {
  Matrix3x4 local_to_world = Matrix3x4::IdentityTransformation();
  Matrix3x4 world_to_local = Matrix3x4::IdentityTransformation();

  Vector3 LocalDirectionToWorld(Vector3 local_direction) {
    return TransformDirection(local_to_world, local_direction);
  }

  Vector3 LocalPositionToWorld(Vector3 local_position) {
    return TransformPosition(local_to_world, local_position);
  }

  Vector3 WorldDirectionToLocal(Vector3 world_direction) {
    return TransformDirection(local_to_world, world_direction);
  }

  Vector3 WorldPositionToLocal(Vector3 world_position) {
    return TransformPosition(local_to_world, world_position);
  }

  OVIS_VM_DECLARE_TYPE_BINDING();
};

}  // namespace ovis
