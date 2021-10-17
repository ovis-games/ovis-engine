#include <charconv>

#include <SDL2/SDL_assert.h>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>

namespace ovis {

SceneObject::SceneObject(Scene* scene, std::string_view name, SceneObject* parent)
    : scene_(scene), parent_(parent), name_(name), path_(BuildPath(name, parent)) {
  SDL_assert(scene_ != nullptr);
  SDL_assert(IsValidName(name_));
  if (parent) {
    parent->children_.push_back(safe_ptr(this));
  }
}

SceneObject::~SceneObject() {
  ClearComponents();
  SDL_assert(components_.size() == 0);
  if (parent_) {
    parent()->children_.erase(parent()->FindChild(name()));
  }
  ClearChildObjects();
}

bool SceneObject::IsValidName(std::string_view name) {
  for (const char c : name) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || (c >= '0' && c <= '9') || c == '.' ||
        c == '-' || c == '#') {
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
  const auto substr_length =
      position_of_last_non_digit != std::string_view::npos ? position_of_last_non_digit + 1 : full_name.size();

  const std::string_view name = full_name.substr(0, substr_length);
  std::optional<unsigned int> number;
  if (substr_length != full_name.size()) {
    const std::string_view number_string = full_name.substr(substr_length);
    unsigned int value;
    auto result = std::from_chars(number_string.data(), number_string.data() + number_string.size(), value);
    SDL_assert(result.ec == std::errc());  // There should be no possibility that there is an error
    number = value;
  }
  return {name, number};
}

bool SceneObject::SetupTemplate(std::string_view template_asset_id) {
  return Deserialize({{"template", std::string(template_asset_id)}});
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
  return child_iterator != children_.end() ? child_iterator->get() : nullptr;
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

SceneObjectComponent* SceneObject::AddComponent(ScriptTypeId component_type) {
  const auto type = global_script_context()->GetType(component_type);
  return AddComponent(type->name);
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

namespace {

std::optional<json> ConstructObjectFromTemplate(std::string_view template_asset) {
  auto asset_library = GetAssetLibraryForAsset(template_asset);

  if (!asset_library) {
    LogE("Invalid scene object template `{}`: asset does not exist", template_asset);
    return {};
  }
  if (auto asset_type = asset_library->GetAssetType(template_asset); asset_type != "scene_object") {
    LogE("Invalid scene object template `{}`: asset has invalid type `{}`", template_asset, asset_type);
    return {};
  }

  const auto object_template_data = asset_library->LoadAssetTextFile(template_asset, "json");
  if (!object_template_data.has_value()) {
    LogE("Invalid scene object template `{}`: asset does not contain json file", template_asset);
    return {};
  }

  const auto object_template = json::parse(*object_template_data);

  if (object_template.contains("template")) {
    const std::string parent_template_asset = object_template.at("template");

    // TODO: detect circular references?
    auto parent_template = ConstructObjectFromTemplate(parent_template_asset);
    if (!parent_template.has_value()) {
      return {};
    }

    parent_template->merge_patch(object_template);
    return *parent_template;
  } else {
    return object_template;
  }
}

}  // namespace

json SceneObject::Serialize() const {
  json serialized_object = json::object();
  json object_template = json::object();

  if (template_.length() > 0) {
    serialized_object["template"] = template_;
    object_template = *ConstructObjectFromTemplate(template_);
  }

  auto& components = serialized_object["components"] = json::object();
  for (const auto& component : components_) {
    const auto serialized_component = component.second->Serialize();

    json::json_pointer component_pointer(fmt::format("/components/{}", component.first));
    if (object_template.contains(component_pointer)) {
      const auto& component_template = object_template[component_pointer];
      json changed_attributes = json::object();

      // TODO: iterate over merged set of template and real object
      for (const auto& attribute : serialized_component.items()) {
        if (attribute.value() != component_template[attribute.key()]) {
          changed_attributes[attribute.key()] = attribute.value();
        }
      }

      components[component.first] = changed_attributes;
    } else {
      components[component.first] = component.second->Serialize();
    }
  }
  auto& children = serialized_object["children"] = json::object();
  for (const auto& child : children_) {
    children[std::string(child->name())] = child->Serialize();
  }
  return serialized_object;
}

bool SceneObject::Deserialize(const json& serialized_object) {
  ClearComponents();
  ClearChildObjects();

  if (serialized_object.contains("template")) {
    template_ = serialized_object.at("template");
    auto object_template = ConstructObjectFromTemplate(template_);
    if (!object_template.has_value()) {
      return false;
    }
    object_template->merge_patch(serialized_object);
    return Update(*object_template);
  } else {
    template_ = "";
    return Update(serialized_object);
  }
}

bool SceneObject::Update(const json& serialized_object) {
  if (!serialized_object.is_object()) {
    return false;
  }

  if (serialized_object.contains("components")) {
    if (!serialized_object["components"].is_object()) {
      LogE("Invalid scene object: components must be an object");
      return false;
    }
    for (const auto& component : serialized_object["components"].items()) {
      if (!SceneObjectComponent::IsRegistered(component.key())) {
        LogE(
            "Scene object deserialization failed: cannot add component `{}` to object. This type has not been "
            "registered.",
            component.key());
        return false;
      }
      if (auto object_component = GetComponent(component.key()); object_component != nullptr) {
        if (!GetComponent(component.key())->Update(component.value())) {
          LogE("Failed to deserialize scene object, could not update component `{}`", component.key());
          return false;
        }
      } else {
        if (auto object_component = AddComponent(component.key());
            object_component == nullptr || !object_component->Deserialize(component.value())) {
          LogE("Failed to deserialize scene object, could not add component `{}`", component.key());
          return false;
        }
      }
    }
  } else {
    LogV("SceneObject deserialization: no 'components' property available!");
  }

  if (serialized_object.contains("children")) {
    if (!serialized_object["children"].is_object()) {
      LogE("Invalid scene object: children must be an object");
      return false;
    }
    for (const auto& child : serialized_object["children"].items()) {
      if (auto child_object = GetChildObject(child.key()); child_object != nullptr) {
        if (!child_object->Update(child.value())) {
          LogE("Failed to deserialize scene object, could not update child object `{}`", child.key());
          return false;
        }
      } else {
        if (CreateChildObject(child.key(), child.value()) == nullptr) {
          LogE("Failed to deserialize scene object, could not add child object `{}`", child.key());
          return false;
        }
      }
    }
  } else {
    LogV("SceneObject deserialization: no 'children' property available!");
  }

  return true;
}

std::vector<safe_ptr<SceneObject>>::const_iterator SceneObject::FindChild(std::string_view name) const {
  return std::find_if(children_.cbegin(), children_.cend(),
                      [name](const safe_ptr<SceneObject>& object) { return object->name() == name; });
}

std::vector<safe_ptr<SceneObject>>::iterator SceneObject::FindChild(std::string_view name) {
  return std::find_if(children_.begin(), children_.end(),
                      [name](const safe_ptr<SceneObject>& object) { return object->name() == name; });
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

void SceneObject::RegisterType(ScriptContext* context) {
  context->RegisterType<SceneObject>("Scene Object");
  context->RegisterFunction<static_cast<SceneObjectComponent* (SceneObject::*)(ScriptTypeId)>(
      &SceneObject::AddComponent)>("scene_object_add_component", { "Object", "Component" });
}

}  // namespace ovis
