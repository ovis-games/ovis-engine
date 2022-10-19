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

namespace {

void ComputeGlobalTransformMatrices(const GlobalTransformMatrices& parent_transform,
                                    const LocalTransformMatrices& local_child_transform,
                                    GlobalTransformMatrices* global_child_transform) {
  global_child_transform->local_to_world = AffineCombine(parent_transform.local_to_world, local_child_transform.local_to_parent);
  global_child_transform->world_to_local = AffineCombine(local_child_transform.parent_to_local, parent_transform.world_to_local);
}

void ComputeGlobalTransformMatricesForChildren(Scene* scene, Entity* parent,
                                               const ComponentStorageView<LocalTransformMatrices>& local_transforms,
                                               ComponentStorageView<GlobalTransformMatrices>* global_transforms) {
  for (auto& child : parent->children(scene)) {
    ComputeGlobalTransformMatrices(
      global_transforms->GetComponent(parent->id),
      local_transforms.GetComponent(child.id),
      &global_transforms->GetComponent(child.id)
    );
    ComputeGlobalTransformMatricesForChildren(scene, &child, local_transforms, global_transforms);
  }
}

}  // namespace

void ComputeGlobalTransformMatrices(Scene* scene,
                                    const ComponentStorageView<LocalTransformMatrices>& local_transforms,
                                    ComponentStorageView<GlobalTransformMatrices>* global_transforms) {
  for (auto& entity : scene->root_entities()) {
    const LocalTransformMatrices& local = local_transforms.GetComponent(entity.id);
    GlobalTransformMatrices& global = global_transforms->GetComponent(entity.id);
    global.world_to_local = local.parent_to_local;
    global.local_to_world = local.local_to_parent;
    ComputeGlobalTransformMatricesForChildren(scene, &entity, local_transforms, global_transforms);
  }
}

}  // namespace ovis
