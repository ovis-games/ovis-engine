#include "ovis/core/scene.hpp"

#include <iterator>
#include <map>
#include <string>

#include "ovis/utils/log.hpp"
#include "ovis/utils/utf8.hpp"
#include "ovis/core/asset_library.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/lua.hpp"
#include "ovis/core/scene_viewport.hpp"

namespace ovis {

Scene::Scene(std::size_t initial_entity_capacity) {
  entities_.reserve(initial_entity_capacity);
  for (std::size_t i = 0; i < initial_entity_capacity; ++i) {
    entities_.push_back(Entity {
      .id = EntityId::CreateInactive(i),
      .parent_id = EntityId::CreateInactive(i),
      .first_children_id = EntityId::CreateInactive(i),
      .previous_sibling_id = EntityId::CreateInactive(i > 0 ? i - 1 : initial_entity_capacity - 1),
      .next_sibling_id = EntityId::CreateInactive((i + 1) % initial_entity_capacity),
    });
  }
  first_inactive_entity_.emplace(EntityId::CreateInactive(0));
}

Scene::~Scene() {
}

Entity* Scene::CreateEntity(std::string_view object_name, std::optional<EntityId> parent_id) {
  assert(!parent_id || IsEntityIdValid(*parent_id));

  // No space left!
  if (!first_inactive_entity_.has_value()) {
    return nullptr;
  }

  Entity* entity = &entities_[first_inactive_entity_->index];
  if (entity->has_siblings()) {
    entities_[entity->previous_sibling_id.index].next_sibling_id = entity->next_sibling_id;
    entities_[entity->next_sibling_id.index].previous_sibling_id = entity->previous_sibling_id;
    first_inactive_entity_ = entity->next_sibling_id;
  } else {
    // This was the last inactive entity
    assert(entity->next_sibling_id == entity->id);
    assert(entity->previous_sibling_id == entity->id);
    first_inactive_entity_.reset();
  }

  entity->id.flags = 1;

  if (parent_id) {
    Entity* parent = GetEntity(*parent_id);
    entity->parent_id = parent->id;
    InsertChild(parent, entity->id);
    // if (parent->first_children_id == parent_id) {
    //   // Parent has no children yet
    //   parent->first_children_id = entity->id;
    // } else {
    //   EntityId current_child_id = parent->first_children_id;
    //   while (current_child_id.index < entity->id.index) {
    //     current_child_id = GetEntity(current_child_id)->next_sibling_id;
    //     if (current_child_id == parent->first_children_id) {
    //       break;
    //     }
    //   }
    // }
  }

  return entity;
}

Entity* Scene::GetEntity(EntityId id) {
  return IsEntityIdValid(id) ? &entities_[id.index] : nullptr;
}

// void Scene::DeleteEntity(EntityId id) {
//   Entity* entity = GetEntityUnchecked(id);
//   assert(entity->is_active());
//   if (entity->has_parent()) {
//   }
//   entities_[id.index].Kill();
// }

// void Scene::ClearEntities() {
//   for (auto& object : entities_) {
//     if (object.is_alive()) {
//       object.Kill();
//     }
//   }
// }

ComponentStorage* Scene::GetComponentStorage(TypeId component_type) {
  for (auto& storage : component_storages_) {
    if (storage.component_type_id() == component_type) {
      return &storage;
    }
  }
  return nullptr;
}

// void Scene::ClearEntities() {
//   while (entities_.size() > 0) {
//     // Destroy one at a time because parent objects will explicitly destory their children
//     // TODO: maybe set a special flag in the scene that we are already destructing?
//     entities_.erase(entities_.begin());
//   }
// }

// Entity* Scene::GetEntity(std::string_view object_path) {
//   auto object = entities_.find(std::string(object_path));
//   if (object == entities_.end()) {
//     return nullptr;
//   }
//   { return object->second.get(); }
// }

// bool Scene::ContainsEntity(std::string_view object_reference) {
//   return entities_.count(std::string(object_reference));
// }
//

void Scene::Prepare() {
  component_storages_.clear();
  frame_scheduler_.Prepare(this);
  {
    const auto object_component_types = frame_scheduler_.GetUsedEntityComponents();
    component_storages_.reserve(object_component_types.size());
    for (const auto component_type : object_component_types) {
      component_storages_.emplace_back(this, component_type, entities_.size());
    }
  }
}

void Scene::Play() {
  // assert(!is_playing());
  // if (!controllers_sorted_) {
  //   SortControllers();
  // }
  // for (const auto& controller : controller_order_) {
  //   controller->Play();
  // }
  is_playing_ = true;
}

void Scene::Stop() {
  // assert(is_playing());
  // if (!controllers_sorted_) {
  //   SortControllers();
  // }
  // for (const auto& controller : controller_order_) {
  //   controller->Stop();
  // }
  // DeleteRemovedControllers();
  is_playing_ = false;
}

void Scene::Update(float delta_time) {
  assert(is_playing() && "Call Play() before calling Update().");
  frame_scheduler_(SceneUpdate{.scene = this, .delta_time = delta_time});
  // if (!controllers_sorted_) {
  //   SortControllers();
  // }
  // for (const auto& controller : controller_order_) {
  //   controller->UpdateWrapper(delta_time);
  // }
}

json Scene::Serialize() const {
  json serialized_object = {{"version", "0.1"}};
//   auto& controllers = serialized_object["controllers"] = json::object();
//   for (const auto& controller : controllers_) {
//     controllers[controller.first] = controller.second->Serialize();
//   }

//   auto& objects = serialized_object["objects"] = json::object();
//   for (const auto& object : entities_) {
//     if (object.second->parent() == nullptr) {
//       objects[object.first] = object.second->Serialize();
//     }
//   }

//   Camera* camera = main_viewport() ? main_viewport()->camera() : nullptr;
//   if (camera != nullptr) {
//     SDL_assert(camera->scene_object() != nullptr);
//     serialized_object["camera"] = camera->scene_object()->name();
//   }
//   // if (camera_object_.size() > 0) {
//   //   serialized_object["camera"] = camera_object_;
//   // }

  return serialized_object;
}

bool Scene::Deserialize(const json& serialized_object) {
//   if (!serialized_object.contains("version") || serialized_object["version"] != "0.1") {
//     LogE("Invalid scene object. Version must be 0.1!");
//     return false;
//   }

//   ClearEntities();
//   ClearControllers();

//   if (serialized_object.contains("controllers") && serialized_object["controllers"].is_object()) {
//     for (const auto& controller : serialized_object["controllers"].items()) {
//       AddController(controller.key(), controller.value());
//     }
//   }

//   if (serialized_object.contains("objects") && serialized_object["objects"].is_object()) {
//     for (const auto& object : serialized_object["objects"].items()) {
//       CreateEntity(object.key(), object.value());
//     }
//   }

//   if (serialized_object.contains("camera")) {
//     // camera_object_ = serialized_object.at("camera");
//     if (main_viewport() != nullptr) {
//       // Need to copy out variable here as nlohmann json currently does not support direct
//       // conversion to std::string_view
//       const std::string camera_object_reference = serialized_object.at("camera");
//       Entity* object = GetEntity(camera_object_reference);
//       if (object != nullptr && object->HasComponent<Camera>()) {
//         main_viewport()->SetCamera(object->GetComponent<Camera>());
//         LogD("Setting Camera");
//       } else {
//         main_viewport()->SetCamera(nullptr);
//         LogD("Setting Camera to null");
//       }
//     }
//   }

 return true;
}

void Scene::InsertChild(Entity* parent, EntityId child_id) {
  assert(false && "Not implmented yet");
}
void Scene::RemoveChild(Entity* parent, EntityId child_id) {
  assert(false && "Not implmented yet");
}

}  // namespace ovis
