#pragma once

#include <type_traits>
#include "ovis/core/main_vm.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/utils/not_null.hpp"
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

  Result<> AddComponent(EntityId entity_id);
  Result<> RemoveComponent(EntityId entity_id);
  void Clear();
  bool EntityHasComponent(EntityId entity_id) const;

  template <typename T>
  T& GetComponent(EntityId id) {
    assert(main_vm->GetTypeId<T>() == component_type_id_);
    assert(flags_[id.index]);
    return *reinterpret_cast<T*>(storage_[id.index]);
  }

  template <typename T>
  const T& GetComponent(EntityId id) const {
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

template <typename T>
class ComponentStorageView {
 public:
  ComponentStorageView(ComponentStorage* storage = nullptr) : storage_(storage) {
    assert(!storage_ || storage_->component_type_id() == main_vm->GetTypeId<T>());
  }

  Scene* scene() const { return storage_->scene(); }
  TypeId component_type_id() const { return storage_->component_type_id(); }
  NotNull<Type*> component_type() const { return storage_->component_type(); }

  Result<> Resize(ContiguousStorage::SizeType size) { return storage_->Resize(size); }

  Result<> AddComponent(EntityId entity_id) { return storage_->AddComponent(entity_id); }
  Result<> RemoveComponent(EntityId entity_id) { return storage_->RemoveComponent(entity_id); }
  bool EntityHasComponent(EntityId entity_id) const { return storage_->EntityHasComponent(entity_id); }

  T& GetComponent(EntityId entity_id) { return storage_->GetComponent<T>(entity_id); }
  const T& GetComponent(EntityId entity_id) const { return storage_->GetComponent<T>(entity_id); }

  T& operator[](EntityId entity_id) { return storage_->GetComponent<T>(entity_id); }
  const T& operator[](EntityId entity_id) const { return storage_->GetComponent<T>(entity_id); }

  operator bool() const {
    return storage_ != nullptr;
  }

 private:
  ComponentStorage* storage_;
};

template <typename T> struct is_component_storage_view : std::false_type {};
template <typename T> struct is_component_storage_view<ComponentStorageView<T>> : std::true_type {};
template <typename T> constexpr bool is_component_storage_view_v = is_component_storage_view<T>::value;

template <typename T> struct component_storage_view_component_type;
template <typename T> struct component_storage_view_component_type<ComponentStorageView<T>> { using type = T; };
template <typename T>
using component_storage_view_component_type_t = typename component_storage_view_component_type<T>::type;

}  // namespace ovis
