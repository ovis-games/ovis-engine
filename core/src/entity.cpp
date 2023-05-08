#include "ovis/core/entity.hpp"

#include "fmt/core.h"
#include <charconv>

#include "ovis/core/scene.hpp"

namespace ovis {

Entity::Siblings Entity::siblings(Scene* scene) {
  return {
    .scene = scene,
    .entity = this,
  };
}

Entity::Children Entity::children(Scene* scene) {
  return {
    .scene = scene,
    .entity = this,
  };
}

Entity::Descendants Entity::descendants(Scene* scene) {
  return {
    .scene = scene,
    .root = this,
  };
}

bool Entity::IsValidName(std::string_view name) {
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

std::pair<std::string_view, std::optional<unsigned int>> Entity::ParseName(std::string_view full_name) {
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
    assert(result.ec == std::errc());  // There should be no possibility that there is an error
    number = value;
  }
  return {name, number};
}

void Entity::SiblingIterator::Increment() {
  entity = scene->GetEntityUnchecked(entity->next_sibling_id);
}

void Entity::AscendingSiblingIterator::Increment() {
  // Detect "wrap aroung"
  entity =
      entity->id.index < entity->next_sibling_id.index ? scene->GetEntityUnchecked(entity->next_sibling_id) : nullptr;
}

namespace {

// TODO: rename
Entity* GoDown(Scene* scene, Entity* entity) {
  return entity->has_children() ? GoDown(scene, scene->GetEntityUnchecked(entity->first_children_id)) : entity;
}

}  // namespace

void Entity::DescendantIterator::Increment() {
  if (current_descendant->has_siblings() && current_descendant->next_sibling_id.index > current_descendant->id.index) {
    current_descendant = GoDown(scene, scene->GetEntityUnchecked(current_descendant->next_sibling_id));
  } else {
    current_descendant = scene->GetEntityUnchecked(current_descendant->parent_id);
  }
}


Entity::SiblingIterator Entity::Siblings::begin() const {
  return {
    .scene = scene,
    .entity = scene->GetEntityUnchecked(entity->next_sibling_id),
  };
}

Entity::SiblingIterator Entity::Siblings::end() const {
  return {
    .scene = scene,
    .entity = entity,
  };
}

Entity::AscendingSiblingIterator Entity::Children::begin() const {
  return {
    .scene = scene,
    .entity = entity->has_children() ? scene->GetEntityUnchecked(entity->first_children_id) : nullptr,
  };
}

Entity::AscendingSiblingIterator Entity::Children::end() const {
  return {
    .scene = scene,
    .entity = nullptr,
  };
}

Entity::DescendantIterator Entity::Descendants::begin() const {
  return {
    .scene = scene,
    .root = root,
    .current_descendant = GoDown(scene, root),
  };
}

Entity::DescendantIterator Entity::Descendants::end() const {
  return {
    .scene = scene,
    .root = root,
    .current_descendant = root,
  };
}

// Result<json> SceneObject::ResolveTemplateForObject(const json& object) {
//   json result_object;
//   if (object.contains("template")) {
//     const auto template_object = LoadTemplate(static_cast<const std::string&>(object.at("template")));
//     OVIS_CHECK_RESULT(template_object);
//     result_object = std::move(**template_object);

//     result_object.merge_patch(object);
//     result_object.erase("template");
//   } else {
//     result_object = object;
//   }

//   if (result_object.contains("children")) {
//     json& children = result_object.at("children");
//     assert(children.is_object());
//     for (auto& [name, child_object] : children.items()) {
//       const auto resolved_child = ResolveTemplateForObject(child_object);
//       OVIS_CHECK_RESULT(resolved_child);
//       child_object = *resolved_child;
//       if (child_object.is_null()) {
//         return Error("Failed to resolve child object {} of template.", name);
//       }
//     }
//   }

//   return result_object;
// }

// Result<const json*> SceneObject::LoadTemplate(std::string_view asset_id) {
//   if (const json* object_template = FindTemplate(asset_id); object_template != nullptr) {
//     if (object_template->is_null()) {
//       return Error("Invalid template detected: {} (this can be caused by circular references)", asset_id);
//     } else {
//       return object_template;
//     }
//   }

//   const AssetLibrary* asset_library = GetAssetLibraryForAsset(asset_id);
//   if (!asset_library) {
//     return Error("Cannot find scene object template: {}", asset_id);
//   }

//   if (const auto asset_type = asset_library->GetAssetType(asset_id); !asset_type || *asset_type != "scene_object") {
//     return Error("Referenced scene object template `{}` has wrong type: {}", asset_id, asset_library->GetAssetType("asset_id"));
//   }

//   const Result<std::string> text_file = asset_library->LoadAssetTextFile(asset_id, "json");
//   OVIS_CHECK_RESULT(text_file);

//   json template_json = json::parse(*text_file);
//   const std::size_t template_index = templates.size();
//   templates.push_back(std::make_pair(std::string(asset_id), json()));

//   const auto resolved_template_json = ResolveTemplateForObject(template_json);
//   OVIS_CHECK_RESULT(resolved_template_json);
//   assert(!resolved_template_json->is_null());
//   assert(templates[template_index].first == asset_id);
//   templates[template_index].second = std::move(*resolved_template_json);
//   return &templates[template_index].second;
// }

OVIS_VM_DEFINE_TYPE_BINDING(Core, Entity) {
  // SceneObject_type->AddProperty<&SceneObject::name>("name");
  // SceneObject_type->AddProperty<&SceneObject::path>("path");
  // SceneObject_type->AddProperty<&SceneObject::parent>("parent");
  // SceneObject_type->AddProperty<&SceneObject::has_parent>("hasParent");

  // SceneObject_type->AddMethod<SelectOverload<SceneObject*(std::string_view)>(&SceneObject::CreateChildObject)>(
  //     "createChildObject");
}

}  // namespace ovis
