#include "ovis/core/transform.hpp"

#include <tuple>

#include "ovis/utils/log.hpp"
#include "ovis/core/matrix.hpp"

namespace ovis {

OVIS_VM_DEFINE_TYPE_BINDING(Core, Transform) {
  Transform_type->attributes.insert("SceneObjectComponent");

  Transform_type->AddProperty<&Transform::position>("position");
  Transform_type->AddProperty<&Transform::rotation>("rotation");
  Transform_type->AddProperty<&Transform::scale>("scale");

}

OVIS_VM_DEFINE_TYPE_BINDING(Core, LocalTransformMatrices) {
  LocalTransformMatrices_type->attributes.insert("SceneObjectComponent");
  LocalTransformMatrices_type->attributes.insert("ComputedSceneObjectComponent");

  LocalTransformMatrices_type->AddProperty<&LocalTransformMatrices::local_to_parent>("localToParent");
  LocalTransformMatrices_type->AddProperty<&LocalTransformMatrices::local_to_parent>("parentToLocal");
}

OVIS_VM_DEFINE_TYPE_BINDING(Core, GlobalTransformMatrices) {
  GlobalTransformMatrices_type->attributes.insert("SceneObjectComponent");
  GlobalTransformMatrices_type->attributes.insert("ComputedSceneObjectComponent");

  GlobalTransformMatrices_type->AddProperty<&GlobalTransformMatrices::local_to_world>("localToWorld");
  GlobalTransformMatrices_type->AddProperty<&GlobalTransformMatrices::world_to_local>("worldToLocal");
}

}  // namespace ovis
