#include "ovis/core/transform.hpp"

#include <tuple>

#include "ovis/utils/log.hpp"
#include "ovis/core/matrix.hpp"

namespace ovis {

OVIS_VM_DEFINE_TYPE_BINDING(Core, Transform) {
  Transform_type->AddAttribute("Core.EntityComponent");

  Transform_type->AddProperty<&Transform::position>("position");
  Transform_type->AddProperty<&Transform::rotation>("rotation");
  Transform_type->AddProperty<&Transform::scale>("scale");
}

OVIS_VM_DEFINE_TYPE_BINDING(Core, LocalTransformMatrices) {
  LocalTransformMatrices_type->AddAttribute("Core.EntityComponent");

  LocalTransformMatrices_type->AddProperty<&LocalTransformMatrices::local_to_parent>("localToParent");
  LocalTransformMatrices_type->AddProperty<&LocalTransformMatrices::local_to_parent>("parentToLocal");
}

OVIS_VM_DEFINE_TYPE_BINDING(Core, GlobalTransformMatrices) {
  GlobalTransformMatrices_type->AddAttribute("Core.EntityComponent");

  GlobalTransformMatrices_type->AddProperty<&GlobalTransformMatrices::local_to_world>("localToWorld");
  GlobalTransformMatrices_type->AddProperty<&GlobalTransformMatrices::world_to_local>("worldToLocal");
}

void ComputeLocalTransformMatrices(const Transform& transform, LocalTransformMatrices* local_transform_matrices) {
  local_transform_matrices->local_to_parent =
      Matrix3x4::FromTransformation(transform.position, transform.scale, transform.rotation);
  local_transform_matrices->parent_to_local = InvertAffine(local_transform_matrices->local_to_parent);
}

}  // namespace ovis
