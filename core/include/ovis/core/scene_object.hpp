#pragma once

#include <cstdint>
#include <string>

#include "ovis/core/vm_bindings.hpp"
#include "ovis/utils/versioned_index.hpp"

namespace ovis {

class Scene;

class SceneObject {
  friend class Scene;

 public:
  using Id = VersionedIndex<uint32_t, 16>;

  SceneObject(Scene* scene, Id id);

  Scene* scene() const { return scene_; }
  Id id() const { return id_; }
  bool is_alive() const { return is_alive_; }
  std::string_view name() const { return name_; }
  std::string_view path() const { return path_; }
  std::optional<Id> parent_id() const { return parent_id_; }
  SceneObject* parent() const;

  static bool IsValidName(std::string_view name);
  static std::pair<std::string_view, std::optional<unsigned int>> ParseName(std::string_view name);

 private:
  Id id_;
  bool is_alive_;
  Scene* scene_;
  std::optional<Id> parent_id_;
  std::set<Id> children_ids_;  // TODO: should use a vector here but I am too lazy right now
  std::string name_;
  std::string path_;

  void Wake(std::string_view name, std::optional<SceneObject::Id> parent = std::nullopt);
  void Kill();

  // ~SceneObject();

  // static std::string BuildPath(std::string_view object_name, SceneObject* parent) {
  //   return parent != nullptr ? fmt::format("{}/{}", parent->path(), object_name) : std::string(object_name);
  // }

  // inline Scene* scene() const { return scene_; }
  // inline std::string_view name() const { return name_; }
  // inline std::string_view path() const { return path_; }
  // inline SceneObject* parent() const { return parent_.get(); }
  // inline bool has_parent() const { return parent_ != nullptr; }

  // void ClearChildObjects();
  // SceneObject* GetChildObject(std::string_view object_name);
  // bool HasChildren() const { return children_.size() > 0; }
  // bool ContainsChildObject(std::string_view object_name);
  // std::vector<safe_ptr<SceneObject>>& children() { return children_; }
  // template <typename T>
  // void ForEachChild(bool recursive, T&& functor);

  // Result<Value> AddComponent(TypeId component_type);
  // template <typename ComponentType> ComponentType* AddComponent();

  // Result<Value> GetComponent(TypeId component_type);
  // Result<Value> GetComponent(TypeId component_type) const;
  // template <typename ComponentType> ComponentType* GetComponent();
  // template <typename ComponentType> const ComponentType* GetComponent() const;

  // // std::span<vm::Value> components() { return components_; }
  // // std::span<const vm::Value> components() const { return components_; }

  // bool HasComponent(TypeId component_type) const;
  // template <typename ComponentType> bool HasComponent() const;

  // auto component_types() const { return TransformRange(components_, [](const auto& component) { return component->type(); }); }
  // auto component_type_ids() const { return TransformRange(components_, [](const auto& component) { return component->type_id(); }); }

  // Result<> RemoveComponent(TypeId component_type);
  // template <typename ComponentType> bool RemoveComponent();
  // void ClearComponents();

  // auto animations() const { return TransformRange(animations_, [](const auto& animation) { return animation.get(); }); }
  // SceneObjectAnimation* GetAnimation(std::string_view name) const;

  // json Serialize() const override;
  // bool Deserialize(const json& serialized_object) override;

  // static void ClearObjectTemplateChache();
  // static SceneObjectAnimation* GetAnimation(std::string_view template_asset_id, std::string_view animation_name);

  // static void RegisterType(sol::table* module);

 // private:
  // std::string path_;
  // std::string name_;
  // std::vector<safe_ptr<SceneObject>> children_;
  // std::vector<std::unique_ptr<Value>> components_;
  // std::vector<safe_ptr<SceneObjectAnimation>> animations_;

  // Result<json> ConstructObjectFromTemplate(std::string_view template_asset, std::span<std::string_view> parents = {}) const;
  // std::vector<safe_ptr<SceneObject>>::const_iterator FindChild(std::string_view name) const;
  // std::vector<safe_ptr<SceneObject>>::iterator FindChild(std::string_view name);

  // static std::vector<std::pair<std::string, json>> templates;
  // static const json* FindTemplate(std::string_view asset_id);
  // static Result<const json*> LoadTemplate(std::string_view asset_id);
  // static Result<json> ResolveTemplateForObject(const json& object);
  // // Maps (scene_object_template, animation_name) -> animation
  // static std::map<std::pair<std::string, std::string>, SceneObjectAnimation, std::less<>> template_animations;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

// template <typename T>
// void SceneObject::ForEachChild(bool recursive, T&& functor) {
//   static_assert(std::is_invocable_v<T, SceneObject*>);
//   for (const auto& child : children()) {
//     functor(child.get_throw());
//     if (recursive) {
//       child->ForEachChild(true, functor);
//     }
//   }
// }

}  // namespace ovis

// namespace std {
// template <>
// struct hash<ovis::SceneObject> {
//   size_t operator()(const ovis::SceneObject& object) const { return hash<std::string_view>()(object.path()); }
// };
// }  // namespace std

// namespace ovis {

// template <typename ComponentType>
// inline ComponentType* SceneObject::AddComponent() {
//   auto component = AddComponent(main_vm->GetTypeId<ComponentType>());
//   return component ? &component->template as<ComponentType>() : nullptr;
// }

// template <typename ComponentType>
// inline ComponentType* SceneObject::GetComponent() {
//   const auto type_id = main_vm->GetTypeId<ComponentType>();
//   for (auto& component : components_) {
//     if (component->type_id() == type_id) {
//       return &component->as<ComponentType>();
//     }
//   }
//   return nullptr;
// }

// template <typename ComponentType>
// inline const ComponentType* SceneObject::GetComponent() const {
//   auto component = GetComponent(main_vm->GetTypeId<ComponentType>());
//   return component ? &component->template as<ComponentType>() : nullptr;
// }

// template <typename ComponentType>
// inline bool SceneObject::HasComponent() const {
//   return HasComponent(main_vm->GetTypeId<ComponentType>());
// }

// template <typename ComponentType>
// inline bool SceneObject::RemoveComponent() { 
//   return RemoveComponent(main_vm->GetTypeId<ComponentType>());
// }

// }  // namespace ovis
