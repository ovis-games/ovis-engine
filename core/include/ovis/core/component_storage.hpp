#pragma once

#include "ovis/core/main_vm.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/contiguous_storage.hpp"
#include "ovis/vm/type_id.hpp"

namespace ovis {

class Scene;

class ComponentStorage {
public:
  ComponentStorage(Scene* scene, TypeId component_type);
  ComponentStorage(Scene* scene, TypeId component_type, ContiguousStorage::SizeType initial_capacity);

  ComponentStorage(ComponentStorage&& other);

  Scene* scene() const { return scene_; }
  TypeId component_type_id() const { return component_type_id_; }
  NotNull<Type*> component_type() const { return main_vm->GetType(component_type_id_); }

  Result<> Resize(ContiguousStorage::SizeType size);

  Result<> AddComponent(Entity::Id object_id);
  Result<> RemoveComponent(Entity::Id object_id);

  template <typename T>
  T& GetComponent(Entity::Id id) {
    assert(main_vm->GetTypeId<T>() == component_type_id_);
    assert(flags_[id.index]);
    return *reinterpret_cast<T*>(storage_[id.index]);
  }

  template <typename T>
  const T& GetComponent(Entity::Id id) const {
    assert(main_vm->GetTypeId<T>() == component_type_id_);
    assert(flags_[id.index]);
    return *reinterpret_cast<const T*>(storage_[id.index]);
  }

private:
  Scene* scene_;
  TypeId component_type_id_;
  ContiguousStorage storage_;
  std::vector<bool> flags_;
};

}  // namespace ovis
