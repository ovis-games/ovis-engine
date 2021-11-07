#pragma once

namespace ovis {

template <typename ComponentType>
inline ComponentType* SceneObject::AddComponent() {
  return down_cast<ComponentType*>(AddComponent(vm::Type::Get<ComponentType>()));
}

template <typename ComponentType>
inline ComponentType* SceneObject::GetComponent() {
  return down_cast<ComponentType*>(GetComponent(vm::Type::Get<ComponentType>()));
}

template <typename ComponentType>
inline const ComponentType* SceneObject::GetComponent() const {
  return down_cast<const ComponentType*>(GetComponent(vm::Type::Get<ComponentType>()));
}

template <typename ComponentType>
inline bool SceneObject::HasComponent() const {
  return HasComponent(vm::Type::Get<ComponentType>());
}

template <typename ComponentType>
inline bool SceneObject::RemoveComponent() { 

  return RemoveComponent(vm::Type::Get<ComponentType>());
}

}
