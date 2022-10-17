#pragma once

#include <memory>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include "ovis/utils/class.hpp"
#include "ovis/utils/down_cast.hpp"
#include "ovis/utils/json.hpp"
#include "ovis/utils/range.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/utils/safe_pointer.hpp"
#include "ovis/utils/serialize.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/core/main_vm.hpp"
#include "ovis/core/vm_bindings.hpp"
#include "ovis/core/scene_object_animation.hpp"
#include "ovis/core/scene_object_component.hpp"

namespace ovis {

class Scene;

class SceneObject : public Serializable, public SafelyReferenceable {
  MAKE_NON_COPYABLE(SceneObject);

 public:
  SceneObject(Scene* scene, std::string_view name, SceneObject* parent = nullptr);
  SceneObject(SceneObject&&) = default;
  ~SceneObject();

  SceneObject& operator=(SceneObject&&) = default;

  static bool IsValidName(std::string_view name);
  static std::pair<std::string_view, std::optional<unsigned int>> ParseName(std::string_view name);
  static std::string BuildPath(std::string_view object_name, SceneObject* parent) {
    return parent != nullptr ? fmt::format("{}/{}", parent->path(), object_name) : std::string(object_name);
  }

  inline Scene* scene() const { return scene_; }
  inline std::string_view name() const { return name_; }
  inline std::string_view path() const { return path_; }
  inline SceneObject* parent() const { return parent_.get(); }
  inline bool has_parent() const { return parent_ != nullptr; }

  SceneObject* CreateChildObject(std::string_view object_name);
  SceneObject* CreateChildObject(std::string_view object_name, const json& serialized_object);
  void DeleteChildObject(std::string_view object_name);
  void DeleteChildObject(SceneObject* object);
  void ClearChildObjects();
  SceneObject* GetChildObject(std::string_view object_name);
  bool HasChildren() const { return children_.size() > 0; }
  bool ContainsChildObject(std::string_view object_name);
  std::vector<safe_ptr<SceneObject>>& children() { return children_; }
  template <typename T>
  void ForEachChild(bool recursive, T&& functor);

  Result<Value> AddComponent(TypeId component_type);
  template <typename ComponentType> ComponentType* AddComponent();

  Result<Value> GetComponent(TypeId component_type);
  Result<Value> GetComponent(TypeId component_type) const;
  template <typename ComponentType> ComponentType* GetComponent();
  template <typename ComponentType> const ComponentType* GetComponent() const;

  // std::span<vm::Value> components() { return components_; }
  // std::span<const vm::Value> components() const { return components_; }

  bool HasComponent(TypeId component_type) const;
  template <typename ComponentType> bool HasComponent() const;

  auto component_types() const { return TransformRange(components_, [](const auto& component) { return component->type(); }); }
  auto component_type_ids() const { return TransformRange(components_, [](const auto& component) { return component->type_id(); }); }

  Result<> RemoveComponent(TypeId component_type);
  template <typename ComponentType> bool RemoveComponent();
  void ClearComponents();

  auto animations() const { return TransformRange(animations_, [](const auto& animation) { return animation.get(); }); }
  SceneObjectAnimation* GetAnimation(std::string_view name) const;

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;

  static void ClearObjectTemplateChache();
  static SceneObjectAnimation* GetAnimation(std::string_view template_asset_id, std::string_view animation_name);

  static void RegisterType(sol::table* module);

 private:
  Scene* scene_;  // No safe_ptr needed, scene is guaranteed to live longer and will (should?) not be moved
  safe_ptr<SceneObject> parent_;
  std::string path_;
  std::string name_;
  std::vector<safe_ptr<SceneObject>> children_;
  std::vector<std::unique_ptr<Value>> components_;
  std::vector<safe_ptr<SceneObjectAnimation>> animations_;

  Result<json> ConstructObjectFromTemplate(std::string_view template_asset, std::span<std::string_view> parents = {}) const;
  std::vector<safe_ptr<SceneObject>>::const_iterator FindChild(std::string_view name) const;
  std::vector<safe_ptr<SceneObject>>::iterator FindChild(std::string_view name);

  static std::vector<std::pair<std::string, json>> templates;
  static const json* FindTemplate(std::string_view asset_id);
  static Result<const json*> LoadTemplate(std::string_view asset_id);
  static Result<json> ResolveTemplateForObject(const json& object);
  // Maps (scene_object_template, animation_name) -> animation
  static std::map<std::pair<std::string, std::string>, SceneObjectAnimation, std::less<>> template_animations;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

template <typename T>
void SceneObject::ForEachChild(bool recursive, T&& functor) {
  static_assert(std::is_invocable_v<T, SceneObject*>);
  for (const auto& child : children()) {
    functor(child.get_throw());
    if (recursive) {
      child->ForEachChild(true, functor);
    }
  }
}

}  // namespace ovis

namespace std {
template <>
struct hash<ovis::SceneObject> {
  size_t operator()(const ovis::SceneObject& object) const { return hash<std::string_view>()(object.path()); }
};
}  // namespace std

namespace ovis {

template <typename ComponentType>
inline ComponentType* SceneObject::AddComponent() {
  auto component = AddComponent(main_vm->GetTypeId<ComponentType>());
  return component ? &component->template as<ComponentType>() : nullptr;
}

template <typename ComponentType>
inline ComponentType* SceneObject::GetComponent() {
  const auto type_id = main_vm->GetTypeId<ComponentType>();
  for (auto& component : components_) {
    if (component->type_id() == type_id) {
      return &component->as<ComponentType>();
    }
  }
  return nullptr;
}

template <typename ComponentType>
inline const ComponentType* SceneObject::GetComponent() const {
  auto component = GetComponent(main_vm->GetTypeId<ComponentType>());
  return component ? &component->template as<ComponentType>() : nullptr;
}

template <typename ComponentType>
inline bool SceneObject::HasComponent() const {
  return HasComponent(main_vm->GetTypeId<ComponentType>());
}

template <typename ComponentType>
inline bool SceneObject::RemoveComponent() { 
  return RemoveComponent(main_vm->GetTypeId<ComponentType>());
}

}  // namespace ovis
