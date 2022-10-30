#include "ovis/core/component_storage.hpp"

#include "ovis/core/main_vm.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/utils/range.hpp"

namespace ovis {

ComponentStorage::ComponentStorage(Scene* scene, TypeId component_type)
    : scene_(scene),
      component_type_id_(component_type),
      storage_(main_vm->GetType(component_type)->memory_layout()) {
}

ComponentStorage::ComponentStorage(Scene* scene, TypeId component_type, ContiguousStorage::SizeType initial_capacity)
    : scene_(scene),
      component_type_id_(component_type),
      storage_(main_vm->GetType(component_type)->memory_layout(), initial_capacity),
      flags_(initial_capacity, false) {
}

ComponentStorage::ComponentStorage(ComponentStorage&& other)
    : scene_(other.scene()), component_type_id_(other.component_type_id()), storage_(other.storage_.memory_layout()) {
  using std::swap;
  swap(storage_, other.storage_);
  swap(flags_, other.flags_);
}

Result<> ComponentStorage::Resize(ContiguousStorage::SizeType size) {
  ContiguousStorage new_storage(component_type()->memory_layout(), size);

  for (std::size_t i = 0; i < size; ++i) {
    if (flags_[i]) {
      Result<> result = new_storage.Construct(i);
      if (result) {
        result = new_storage.CopyTo(i, storage_[i]);
      }
      if (!result) {
        // If something failed destruct all constructed objects
        for (std::size_t j = 0; j < i; ++j) {
          if (flags_[j]) {
            new_storage.Destruct(i);
          }
        }
        return result;
      }
    }
  }
  flags_.resize(size, false);
  swap(storage_, new_storage);

  return Success;
}

Result<> ComponentStorage::AddComponent(EntityId object_id) {
  if (!scene()->IsEntityIdValid(object_id)) {
    return Error("Invalid entity id");
  }

  if (flags_[object_id.index]) {
    return Error("Entity already has component {}", component_type()->name());
  }

  OVIS_CHECK_RESULT(storage_.Construct(object_id.index));
  flags_[object_id.index] = true;
  return Success;
}

Result<> ComponentStorage::RemoveComponent(EntityId object_id) {
  if (!scene()->IsEntityIdValid(object_id)) {
    return Error("Invalid entity id");
  }

  if (!flags_[object_id.index]) {
    return Error("Entity does not have the component {}", component_type()->name());
  }

  storage_.Destruct(object_id.index);
  flags_[object_id.index] = false;
  return Success;
}

void ComponentStorage::Clear() {
  for (auto i : IRange(this->flags_.size())) {
    if (flags_[i]) {
      storage_.DestructRange(i, 1);
      flags_[i] = false;
    }
  }
}

bool ComponentStorage::EntityHasComponent(EntityId entity_id) const {
  assert(scene()->IsEntityIdValid(entity_id));
  return flags_[entity_id.index];
}

}  // namespace ovis
