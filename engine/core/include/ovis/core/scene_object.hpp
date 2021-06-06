#pragma once

#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <string_view>

#include <ovis/utils/class.hpp>
#include <ovis/utils/down_cast.hpp>
#include <ovis/utils/json.hpp>
#include <ovis/utils/safe_pointer.hpp>
#include <ovis/utils/serialize.hpp>
#include <ovis/core/scene_object_component.hpp>

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
  SceneObject* CreateChildObject(std::string_view object_name, const sol::table& properties);
  void DeleteChildObject(std::string_view object_name);
  void DeleteChildObject(SceneObject* object);
  void ClearChildObjects();
  SceneObject* GetChildObject(std::string_view object_name);
  bool HasChildren() const { return children_.size() > 0; }
  bool ContainsChildObject(std::string_view object_name);
  std::vector<safe_ptr<SceneObject>>& children() { return children_; }
  template <typename T> void ForEachChild(bool recursive, T&& functor);

  template <typename ComponentType = SceneObjectComponent>
  inline ComponentType* AddComponent(const std::string& component_id) {
    return down_cast<ComponentType*>(AddComponent(component_id));
  }

  SceneObjectComponent* AddComponent(const std::string& component_id);
  SceneObjectComponent* AddComponent(const std::string& component_id, const sol::table& component_properties);
  bool HasComponent(const std::string& component_id) const;

  template <typename ComponentType = SceneObjectComponent>
  ComponentType* GetComponent(const std::string& component_id) {
    auto component = components_.find(component_id);
    if (component == components_.end()) {
      return nullptr;
    } else {
      return down_cast<ComponentType*>(component->second.get());
    }
  }

  template <typename ComponentType = SceneObjectComponent>
  const ComponentType* GetComponent(const std::string& component_id) const {
    auto component = components_.find(component_id);
    if (component == components_.end()) {
      return nullptr;
    } else {
      return down_cast<ComponentType*>(component->second.get());
    }
  }

  void GetComponentIds(std::vector<std::string>* component_ids) const;
  inline std::vector<std::string> GetComponentIds() const {
    std::vector<std::string> component_ids;
    GetComponentIds(&component_ids);
    return component_ids;
  }
  void RemoveComponent(const std::string& component_id);
  void ClearComponents();

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;

  static void RegisterType(sol::table* module);

 private:
  Scene* scene_; // No safe_ptr needed, scene is guaranteed to live longer and will (should?) not be moved
  safe_ptr<SceneObject> parent_;
  std::string path_;
  std::string name_;
  std::vector<safe_ptr<SceneObject>> children_;
  std::unordered_map<std::string, std::unique_ptr<SceneObjectComponent>> components_;

  std::vector<safe_ptr<SceneObject>>::const_iterator FindChild(std::string_view name) const;
  std::vector<safe_ptr<SceneObject>>::iterator FindChild(std::string_view name);
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
  template <> struct hash<ovis::SceneObject>
  {
    size_t operator()(const ovis::SceneObject& object) const
    {
      return hash<std::string_view>()(object.path());
    }
  };
}
