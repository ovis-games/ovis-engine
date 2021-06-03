#include <charconv>
#include <SDL2/SDL_assert.h>

#include <ovis/utils/log.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>

namespace ovis {

SceneObject::SceneObject(Scene* scene, std::string_view name, SceneObject* parent) : scene_(scene), parent_(parent), name_(name), path_(BuildPath(name, parent)) {
  SDL_assert(scene_ != nullptr);
  SDL_assert(IsValidName(name_));
  if (parent) {
    parent->children_.push_back(safe_ptr(this));
  }
}

SceneObject::~SceneObject() {
  auto component_ids = GetComponentIds();
  for (const auto& component_id : component_ids) {
    RemoveComponent(component_id);
  }
  SDL_assert(components_.size() == 0);
  if (parent_) {
    parent()->children_.erase(parent()->FindChild(name()));
  }
  ClearChildObjects();
}

bool SceneObject::IsValidName(std::string_view name) {
  for (const char c : name) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || (c >= '0' && c <= '9') || c == '.' || c == '-') {
      // Character is allowed
      continue;
    }
    // Character is not allowed
    return false;
  }
  return true;
}

std::pair<std::string_view, std::optional<unsigned int>> SceneObject::ParseName(std::string_view full_name) {
  using std::operator""sv;
  const auto position_of_last_non_digit = full_name.find_last_not_of("1234567890"sv);
  const auto substr_length = position_of_last_non_digit != std::string_view::npos ?
    position_of_last_non_digit + 1 :
    full_name.size();

  const std::string_view name = full_name.substr(0, substr_length);
  std::optional<unsigned int> number;
  if (substr_length != full_name.size()) {
    const std::string_view number_string = full_name.substr(substr_length);
    unsigned int value;
    auto result = std::from_chars(number_string.data(), number_string.data() + number_string.size(), value);
    SDL_assert(result.ec == std::errc()); // There should be no possibility that there is an error
    number = value;
  }
  return { name, number };
}

SceneObject* SceneObject::CreateChildObject(std::string_view object_name) {
  return scene_->CreateObject(object_name, this);
}

SceneObject* SceneObject::CreateChildObject(std::string_view object_name, const json& serialized_object) {
  return scene_->CreateObject(object_name, serialized_object, this);
}

SceneObject* SceneObject::CreateChildObject(std::string_view object_name, const sol::table& component_properties) {
  return scene_->CreateObject(object_name, component_properties, this);
}

void SceneObject::DeleteChildObject(std::string_view object_name) {
  auto child_iterator = FindChild(object_name);
  if (child_iterator != children_.end()) {
    scene_->DeleteObject(child_iterator->get());
  }
}

void SceneObject::ClearChildObjects() {
  while (children().size() > 0) {
    scene()->DeleteObject(children().back().get());
  }
}

SceneObject* SceneObject::GetChildObject(std::string_view object_name) {
  const auto child_iterator = FindChild(object_name);
  return child_iterator->get();
}

bool SceneObject::ContainsChildObject(std::string_view object_name) {
  return FindChild(object_name) != children_.end();
}

SceneObjectComponent* SceneObject::AddComponent(const std::string& component_id) {
  if (HasComponent(component_id)) {
    LogE("Object '{}' already has the component '{}'.", name_, component_id);
    return nullptr;
  } else {
    std::optional<std::unique_ptr<SceneObjectComponent>> component = SceneObjectComponent::Create(component_id, this);
    if (!component.has_value()) {
      LogE("Component '{}' not registered or failed to create", component_id);
      return nullptr;
    } else {
      SDL_assert(*component != nullptr);
      return components_.insert(std::make_pair(component_id, std::move(*component))).first->second.get();
    }
  }
}

SceneObjectComponent* SceneObject::AddComponent(const std::string& component_id, const sol::table& properties) {
  SceneObjectComponent* component = AddComponent(component_id);

  if (component != nullptr) {
    sol::table lua_component = component->GetValue().as<sol::table>();
    for (const auto& [key, value] : properties) {
      lua_component[key] = value;
    }
  }

  return component;
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

void SceneObject::ClearComponents() {
  components_.clear();
}

json SceneObject::Serialize() const {
  json serialized_object = json::object();
  auto& components = serialized_object["components"] = json::object();
  for (const auto& component : components_) {
    components[component.first] = component.second->Serialize();
  }
  auto& children = serialized_object["children"] = json::object();
  for (const auto& child : children_) {
    children[std::string(child->name())] = child->Serialize();
  }
  return serialized_object;
}

bool SceneObject::Deserialize(const json& serialized_object) {
  if (!serialized_object.is_object()) {
    return false;
  }
  ClearComponents();
  ClearChildObjects();

  if (serialized_object.contains("components")) {
    if (!serialized_object["components"].is_object()) {
      return false;
    }
    for (const auto& component : serialized_object["components"].items()) {
      if (!SceneObjectComponent::IsRegistered(component.key())) {
        return false;
      }
      AddComponent(component.key())->Deserialize(component.value());
    }
  } else {
    LogV("SceneObject deserialization: no 'components' property available!");
  }

  if (serialized_object.contains("children")) {
    if (!serialized_object["children"].is_object()) {
      return false;
    }
    for (const auto& child : serialized_object["children"].items()) {
      CreateChildObject(child.key(), child.value());
    }
  } else {
    LogV("SceneObject deserialization: no 'children' property available!");
  }
  return true;
}

std::vector<safe_ptr<SceneObject>>::const_iterator SceneObject::FindChild(std::string_view name) const {
  return std::find_if(children_.cbegin(), children_.cend(), [name](const safe_ptr<SceneObject>& object) {
    return object->name() == name;
  });
}

std::vector<safe_ptr<SceneObject>>::iterator SceneObject::FindChild(std::string_view name) {
  return std::find_if(children_.begin(), children_.end(), [name](const safe_ptr<SceneObject>& object) {
    return object->name() == name;
  });
}

void SceneObject::RegisterType(sol::table* module) {
  /// Represents an object in a scene
  // @classmod ovis.core.SceneObject
  // @testinginclude <ovis/core/scene.hpp>
  // @cppsetup ovis::Scene scene;
  // @cppsetup ovis::lua["some_scene"] = &scene;
  // @usage local core = require "ovis.core"
  // local some_object = some_scene:add_object("Some Object")
  sol::usertype<SceneObject> scene_object_type = module->new_usertype<SceneObject>("SceneObject", sol::no_constructor);

  /// The name of the object.
  // Names of objects with the same parent must be unique.
  // @see 02-scene-structure.md
  // @usage assert(some_object.name == "Some Object")
  scene_object_type["name"] = sol::property(&SceneObject::name);

  /// Adds a component to the object.
  // @function add_component
  // @param id The id of the component.
  // @return The newly added component
  // @usage local transform = some_object:add_component("Transform")
  // assert(transform ~= nil)
  scene_object_type["add_component"] = sol::overload(
      [](SceneObject* object, const std::string& component_id) {
        SceneObjectComponent* component = object->AddComponent(component_id);
        return component ? component->GetValue() : nullptr;
      },
      [](SceneObject* object, const std::string& component_id, const sol::table& properties) -> sol::lua_value {
        SceneObjectComponent* component = object->AddComponent(component_id, properties);
        return component ? component->GetValue() : nullptr;
      });

  /// Checks whether a component is attached to an object.
  // @function has_component
  // @param id The id of the component.
  // @treturn bool
  // @usage assert(not some_object:has_component("Transform"))
  // some_object:add_component("Transform")
  // assert(some_object:has_component("Transform"))
  scene_object_type["has_component"] = &SceneObject::HasComponent;

  /// Removes a component from the object.
  // @function remove_component
  // @param id The id of the component to remove.
  // @usage assert(not some_object:has_component("Transform"))
  // some_object:add_component("Transform")
  // assert(some_object:has_component("Transform"))
  // some_object:remove_component("Transform")
  // assert(not some_object:has_component("Transform"))
  scene_object_type["remove_component"] = &SceneObject::RemoveComponent;

  /// Returns a component.
  // @function get_component
  // @param id The id of the component.
  // @usage local transform = some_object:get_component("Transform")
  // assert(transform == nil)
  // some_object:add_component("Transform")
  // transform = some_object:get_component("Transform")
  // assert(transform ~= nil)
  scene_object_type["get_component"] = [](SceneObject* object, const std::string& component_id) {
    SceneObjectComponent* component = object->GetComponent(component_id);
    return component ? component->GetValue() : nullptr;
  };
}

}  // namespace ovis
