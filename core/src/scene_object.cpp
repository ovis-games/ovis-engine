#include <charconv>

#include <SDL2/SDL_assert.h>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_animation.hpp>

namespace ovis {

std::vector<std::pair<std::string, json>> SceneObject::templates;
std::map<std::pair<std::string, std::string>, SceneObjectAnimation, std::less<>> SceneObject::template_animations;

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

SceneObject* SceneObject::CreateChildObject(std::string_view object_name) {
  return scene_->CreateObject(object_name, this);
}

SceneObject* SceneObject::CreateChildObject(std::string_view object_name, const json& serialized_object) {
  return scene_->CreateObject(object_name, serialized_object, this);
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

vm::Value SceneObject::AddComponent(const std::shared_ptr<vm::Type>& type) {
  if (!type) {
    LogE("Invalid object component");
    return vm::Value::None();
  }
  if (!type->IsDerivedFrom<SceneObjectComponent>()) {
    LogE("{} does not derived from SceneObjectComponent", type->full_reference());
    return vm::Value::None();
  }

  if (HasComponent(type)) {
    LogE("Object '{}' already has the component '{}'.", path(), type->name());
    return vm::Value::None();
  } else {
    auto component = SceneObjectComponent::Create(std::string(type->full_reference()), this);
    if (component.has_value()) {
      components_.push_back({
        .type = type,
        .pointer = std::move(*component),
      });
      return vm::Value::CreateView(components_.back().pointer.get(), type);
    } else {
      LogE("Failed to construct component");
      return vm::Value::None();
    }
  }
}

vm::Value SceneObject::GetComponent(const std::shared_ptr<vm::Type>& type) {
  for (const auto& component : components_) {
    if (component.type.lock() == type) {
      return vm::Value::CreateView(component.pointer.get(), type);
    }
  }
  return vm::Value::None();
}

vm::Value SceneObject::GetComponent(const std::shared_ptr<vm::Type>& type) const {
  for (const auto& component : components_) {
    if (component.type.lock() == type) {
      return vm::Value::CreateView(component.pointer.get(), type);
    }
  }
  return vm::Value::None();
}

bool SceneObject::HasComponent(const std::shared_ptr<vm::Type>& type) const {
  for (const auto& component : components_) {
    if (component.type.lock() == type) {
      return true;
    }
  }
  return false;
}

bool SceneObject::RemoveComponent(const std::shared_ptr<vm::Type>& type) {
  const auto erased_count = std::erase_if(components_, [type](const auto& component) {
      return component.type.lock() == type;
  });
  assert(erased_count <= 1);
  return erased_count > 0;
}

void SceneObject::ClearComponents() {
  components_.clear();
}

SceneObjectAnimation* SceneObject::GetAnimation(std::string_view name) const {
  for (const auto& animation : animations_) {
    if (animation->name() == name) {
      return animation.get();
    }
  }
  return nullptr;
}

json SceneObject::Serialize() const {
  json serialized_object = json::object();


  auto& components = serialized_object["components"] = json::object();
  for (const auto& component : components_) {
    const auto component_type = component.type.lock();
    assert(component_type != nullptr);
    components[std::string(component_type->full_reference())] = component.pointer->Serialize();
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

  const Result<json> object_json = ResolveTemplateForObject(serialized_object);
  if (!object_json) {
    // Can happen if the template is invalid. E.g., it does not exist or contains circular references
    LogE("Failed to deserialize scene object");
    return false;
  }

  if (object_json->contains("components")) {
    const json& components = object_json->at("components");
    assert(components.is_object());
    for (const auto& [component_id, component_json] : components.items()) {
      if (!SceneObjectComponent::IsRegistered(component_id)) {
        LogE(
            "Scene object deserialization failed: cannot add component `{}` to object. This type has not been "
            "registered.",
            component_id);
        ClearComponents();
        return false;
      }
      const auto type = vm::Type::Deserialize(component_id);
      const vm::Value component = AddComponent(type);
      if (!component.Get<SceneObjectComponent*>()->Deserialize(component_json)) {
        LogE("Failed to deserialize scene object, could not deserialize `{}`", component_id);
        ClearComponents();
        return false;
      }
    }
  }

  if (object_json->contains("children")) {
    const auto& children = object_json->at("children");
    assert(children.is_object());
    for (const auto& [child_name, child_json] : children.items()) {
      assert(!ContainsChildObject(child_name));
      if (CreateChildObject(child_name, child_json) == nullptr) {
        LogE("Failed to deserialize scene object, could not add child object `{}`", child_name);
        ClearComponents();
        ClearChildObjects();
        return false;
      }
    }
  }

  return true;
}

SceneObjectAnimation* SceneObject::GetAnimation(std::string_view template_asset_id, std::string_view animation_name) {
  auto animation_it = template_animations.find(std::make_pair(std::string(template_asset_id), std::string(animation_name)));
  if (animation_it != template_animations.end()) {
    return &animation_it->second;
  } else {
    return nullptr;
  }
}

Result<json> SceneObject::ConstructObjectFromTemplate(std::string_view template_asset, std::span<std::string_view> parents) const {
  auto asset_library = GetAssetLibraryForAsset(template_asset);

  if (!asset_library) {
    return Error("Invalid scene object template `{}`: asset does not exist", template_asset);
  }
  if (auto asset_type = asset_library->GetAssetType(template_asset); !asset_type || *asset_type != "scene_object") {
    OVIS_CHECK_RESULT(asset_type);
    return Error("Invalid scene object template `{}`: asset has invalid type `{}`", template_asset, asset_type);
  }

  const auto object_template_data = asset_library->LoadAssetTextFile(template_asset, "json");
  OVIS_CHECK_RESULT(object_template_data);

  auto object_template = json::parse(*object_template_data);

  if (object_template.contains("template")) {
    const std::string parent_template_asset = object_template.at("template");

    for (const auto& parent : parents) {
      if (parent == parent_template_asset) {
        // Circular reference
        return Error("Object template {} contains a circular reference", template_asset);
      }
    }

    std::vector<std::string_view> parents_for_children;
    parents_for_children.reserve(parents.size() + 1);
    parents_for_children.insert(parents_for_children.end(), parents.begin(), parents.end());
    parents_for_children.push_back(template_asset);

    auto parent_template = ConstructObjectFromTemplate(parent_template_asset, parents_for_children);
    OVIS_CHECK_RESULT(parent_template);

    json animations = parent_template->contains("animations") ? parent_template->at("animations") : json::object();
    assert(animations.is_object());
    if (object_template.contains("animations")) {
      for (const auto& [name, animation] : object_template["animations"].items()) {
        assert(animation.is_array());
        assert(animation.size() == 1);
        if (animations.contains(name)) {
          assert(animations[name].is_array());
          animations[name].push_back(animation[0]);
        } else {
          animations[name] = animation;
        }
      }
    }
    object_template["animations"] = std::move(animations);

    parent_template->merge_patch(object_template);
    object_template = *parent_template;
  }

  if (object_template.contains("animation")) {
    for (auto& [name, animation_value] : object_template["animations"].items()) {
      std::pair<std::string, std::string> animation_identifier = std::make_pair(std::string(template_asset), name);
      if (!template_animations.contains(animation_identifier)) {
        SceneObjectAnimation animation(name);
        if (!animation.Deserialize(animation_value)) {
          LogE("Failed to deserialize animation");
        } else {
          template_animations.insert(std::make_pair(std::move(animation_identifier), std::move(animation)));
        }
      }
    }
  }
  
  return object_template;
}

std::vector<safe_ptr<SceneObject>>::const_iterator SceneObject::FindChild(std::string_view name) const {
  return std::find_if(children_.cbegin(), children_.cend(),
                      [name](const safe_ptr<SceneObject>& object) { return object->name() == name; });
}

std::vector<safe_ptr<SceneObject>>::iterator SceneObject::FindChild(std::string_view name) {
  return std::find_if(children_.begin(), children_.end(),
                      [name](const safe_ptr<SceneObject>& object) { return object->name() == name; });
}

void SceneObject::ClearObjectTemplateChache() {
  templates.clear();
}

void SceneObject::RegisterType(sol::table* module) {
}

const json* SceneObject::FindTemplate(std::string_view asset_id) {
  for (const auto& object_template : templates) {
    if (object_template.first == asset_id) {
      return &object_template.second;
    }
  }
  return nullptr;
}

Result<json> SceneObject::ResolveTemplateForObject(const json& object) {
  json result_object;
  if (object.contains("template")) {
    const auto template_object = LoadTemplate(static_cast<const std::string&>(object.at("template")));
    OVIS_CHECK_RESULT(template_object);
    result_object = std::move(**template_object);

    result_object.merge_patch(object);
    result_object.erase("template");
  } else {
    result_object = object;
  }

  if (result_object.contains("children")) {
    json& children = result_object.at("children");
    assert(children.is_object());
    for (auto& [name, child_object] : children.items()) {
      const auto resolved_child = ResolveTemplateForObject(child_object);
      OVIS_CHECK_RESULT(resolved_child);
      child_object = *resolved_child;
      if (child_object.is_null()) {
        return Error("Failed to resolve child object {} of template.", name);
      }
    }
  }

  return result_object;
}

Result<const json*> SceneObject::LoadTemplate(std::string_view asset_id) {
  if (const json* object_template = FindTemplate(asset_id); object_template != nullptr) {
    if (object_template->is_null()) {
      return Error("Invalid template detected: {} (this can be caused by circular references)", asset_id);
    } else {
      return object_template;
    }
  }

  const AssetLibrary* asset_library = GetAssetLibraryForAsset(asset_id);
  if (!asset_library) {
    return Error("Cannot find scene object template: {}", asset_id);
  }

  if (const auto asset_type = asset_library->GetAssetType(asset_id); !asset_type || *asset_type != "scene_object") {
    return Error("Referenced scene object template `{}` has wrong type: {}", asset_id, asset_library->GetAssetType("asset_id"));
  }

  const Result<std::string> text_file = asset_library->LoadAssetTextFile(asset_id, "json");
  OVIS_CHECK_RESULT(text_file);

  json template_json = json::parse(*text_file);
  const std::size_t template_index = templates.size();
  templates.push_back(std::make_pair(std::string(asset_id), json()));

  const auto resolved_template_json = ResolveTemplateForObject(template_json);
  OVIS_CHECK_RESULT(resolved_template_json);
  assert(!resolved_template_json->is_null());
  assert(templates[template_index].first == asset_id);
  templates[template_index].second = std::move(*resolved_template_json);
  return &templates[template_index].second;
}

}  // namespace ovis
