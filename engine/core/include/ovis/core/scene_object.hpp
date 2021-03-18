#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include <ovis/core/class.hpp>
#include <ovis/core/down_cast.hpp>
#include <ovis/core/json.hpp>
#include <ovis/core/serialize.hpp>
#include <ovis/scene/scene_object_component.hpp>

namespace ovis {

class Scene;

class SceneObject : public Serializable {
  MAKE_NON_COPYABLE(SceneObject);

 public:
  SceneObject(Scene* scene, const std::string& name);
  ~SceneObject();

  inline Scene* scene() const { return scene_; }
  inline std::string name() const { return name_; }

  template <typename ComponentType = SceneObjectComponent>
  inline ComponentType* AddComponent(const std::string& component_id) {
    return down_cast<ComponentType*>(AddComponent(component_id));
  }

  SceneObjectComponent* AddComponent(const std::string& component_id);
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
  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<SceneObjectComponent>> components_;
};

}  // namespace ovis
