#include <SDL2/SDL_assert.h>

#include <ovis/core/log.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_object.hpp>

namespace ovis {

SceneObject::SceneObject(Scene* scene, const std::string& name) : scene_(scene), name_(name) {
  SDL_assert(scene_ != nullptr);
}

SceneObject::~SceneObject() {}

SceneObjectComponent* SceneObject::AddComponent(const std::string& component_id) {
  if (HasComponent(component_id)) {
    LogE("Object '{}' already has the component '{}'.", name_, component_id);
    return nullptr;
  } else {
    auto factory = Module::scene_object_component_factory_functions()->find(component_id);
    if (factory == Module::scene_object_component_factory_functions()->end()) {
      LogE("Component not registered: '{}'", component_id);
      return nullptr;
    } else {
      auto component = factory->second(this);
      SDL_assert(component != nullptr);
      component->scene_object_ = this;
      return components_.insert(std::make_pair(component_id, std::move(component))).first->second.get();
    }
  }
}

bool SceneObject::HasComponent(const std::string& component_id) const {
  return components_.count(component_id) != 0;
}

void SceneObject::GetComponentIds(std::vector<std::string>* component_ids) const {
  SDL_assert(component_ids != nullptr);

  component_ids->clear();
  component_ids->reserve(components_.size());
  for (const auto& component : components_) {
    component_ids->push_back(component.first);
  }
}

void SceneObject::RemoveComponent(const std::string& component_id) {
  if (components_.erase(component_id) == 0) {
    LogE("Failed to erase component '{}' from '{}': component does not exist.");
  }
}

json SceneObject::Serialize() const {
  json serialized_object = json::object();
  auto& components = serialized_object["components"] = json::object();
  for (const auto& component : components_) {
    components[component.first] = component.second->Serialize();
  }
  return serialized_object;
}

bool SceneObject::Deserialize(const json& serialized_object) {
  if (!serialized_object.is_object()) {
    return false;
  }
  if (serialized_object.contains("components")) {
    if (!serialized_object["components"].is_object()) {
      return false;
    }
    for (const auto& component : serialized_object["components"].items()) {
      if (std::find(SceneObjectComponent::GetRegisteredComponents().begin(),
                    SceneObjectComponent::GetRegisteredComponents().end(),
                    component.key()) == SceneObjectComponent::GetRegisteredComponents().end()) {
        return false;
      }
      AddComponent(component.key())->Deserialize(component.value());
    }
  } else {
    LogV("SceneObject deserialization: no 'components' property available!");
  }
  return true;
}

int SceneObject::LoadLuaModule(lua_State* l) {
  sol::state_view lua(l);

  /// Represents an object in a scene
  // @classmod SceneObject

  sol::usertype<SceneObject> scene_object_type =
      Lua::state.new_usertype<SceneObject>("SceneObject", "components",
                                           sol::property(&Module::SceneObjectComponentsWrapper::FromObject));
  
  /// The name of the object.
  // Names of objects with the same parent must be unique.
  // @see 02-scene-structure.md
  scene_object_type["name"] = sol::property(&SceneObject::name);

  /// Adds a component to the object.
  // It is a specialized splitting operation on a string.
  // @function add_component
  // @param name The id of the component (see component_ids)
  // @return The newly added component
  scene_object_type["AddComponent"] =
      static_cast<SceneObjectComponent* (SceneObject::*)(const std::string&)>(&SceneObject::AddComponent);
  scene_object_type["HasComponent"] = &SceneObject::HasComponent;

  return scene_object_type.push();
}

}  // namespace ovis
