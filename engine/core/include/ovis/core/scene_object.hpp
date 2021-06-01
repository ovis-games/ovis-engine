#pragma once

#include <memory>
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
  SceneObject(Scene* scene, const std::string& name, SceneObject* parent = nullptr);
  SceneObject(SceneObject&&) = default;
  ~SceneObject();

  SceneObject& operator=(SceneObject&&) = default;

  static bool IsValidName(std::string_view name);

  inline Scene* scene() const { return scene_; }
  inline std::string name() const { return name_; }
  inline SceneObject* parent() const { return parent_.get(); }

  SceneObject* CreateChildObject(std::string_view object_name);
  SceneObject* CreateChildObject(std::string_view object_name, const json& serialized_object);
  SceneObject* CreateChildObject(std::string_view object_name, const sol::table& properties);
  void DeleteChildObject(std::string_view object_name);
  void DeleteChildObject(SceneObject* object);
  void ClearChildObjects();
  SceneObject* GetChildObject(std::string_view object_name);
  bool HasChildren() const { return children_.size() > 0; }
  bool ContainsChildObject(std::string_view object_name);
  std::vector<SceneObject>& children() { return children_; }
  std::string GetPath() const { return parent_ ? fmt::format("{}/{}", parent_->GetPath(), name()) : name(); }

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

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;

  static void RegisterType(sol::table* module);

 private:
  Scene* scene_;
  safe_ptr<SceneObject> parent_;
  std::string name_;
  std::vector<SceneObject> children_;
  std::unordered_map<std::string, std::unique_ptr<SceneObjectComponent>> components_;

  std::vector<SceneObject>::const_iterator FindChild(std::string_view name) const;
  std::vector<SceneObject>::iterator FindChild(std::string_view name);
};

}  // namespace ovis
