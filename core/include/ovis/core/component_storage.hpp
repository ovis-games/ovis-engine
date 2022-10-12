#pragma once

#include "ovis/core/main_vm.hpp"
#include "ovis/core/scene_object.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/contiguous_storage.hpp"
#include "ovis/vm/type_id.hpp"

namespace ovis {

class Scene;

class ComponentStorage {
public:
  ComponentStorage(Scene* scene, TypeId component_type);

  Scene* scene() const { return scene_; }
  TypeId component_type() const { return component_type_; }

  Result<> Resize(ContiguousStorage::SizeType size);

  Result<> AddComponent(SceneObject::Id object_id);
  Result<> RemoveComponent(SceneObject::Id object_id);

  template <typename T>
  T& GetComponent(SceneObject::Id id) {
    assert(main_vm->GetTypeId<T>() == component_type_);
    assert(flags_[id.index]);
    return *reinterpret_cast<T*>(storage_[id.index]);
  }

  template <typename T>
  const T& GetComponent(SceneObject::Id id) const {
    assert(main_vm->GetTypeId<T>() == component_type_);
    assert(flags_[id.index]);
    return *reinterpret_cast<const T*>(storage_[id.index]);
  }

private:
  Scene* scene_;
  TypeId component_type_;
  ContiguousStorage storage_;
  std::vector<bool> flags_;
};

}  // namespace ovis
