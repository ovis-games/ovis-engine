#include "ovis/core/scene.hpp"

#include <iterator>
#include <map>
#include <string>

#include "ovis/utils/log.hpp"
#include "ovis/utils/utf8.hpp"
#include "ovis/core/asset_library.hpp"
#include "ovis/core/entity.hpp"
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
    first_inactive_entity_ = entity->next_sibling_id;
    RemoveSibling(entity->id);
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
    if (parent->has_children()) {
      parent->first_children_id = InsertSibling(parent->first_children_id, entity->id);
    } else {
      parent->first_children_id = entity->id;
      entity->next_sibling_id = EntityId::CreateInactive(entity->id.index);
      entity->previous_sibling_id = EntityId::CreateInactive(entity->id.index);
    }
  } else {
    parent_id = EntityId::CreateInactive(entity->id.index);
    if (first_active_entity_.has_value()) {
      assert(last_active_entity_.has_value());
      first_active_entity_ = InsertSibling(*first_active_entity_, entity->id);
      if (entity->id.index > last_active_entity_->index) {
        last_active_entity_ = entity->id;
      }
    } else {
      entity->next_sibling_id = EntityId::CreateInactive(entity->id.index);
      entity->previous_sibling_id = EntityId::CreateInactive(entity->id.index);
      first_active_entity_ = entity->id;
      last_active_entity_ = entity->id;
    }
  }

  return entity;
}

Entity* Scene::GetEntity(EntityId id) {
  return IsEntityIdValid(id) ? &entities_[id.index] : nullptr;
}

Entity* Scene::GetEntityUnchecked(EntityId id) {
  assert(id.index < entities_.size());
  assert(entities_[id.index].id == id);
  return &entities_[id.index];
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


Scene::EntityIterator Scene::begin() {
  return {
    .scene = this,
    .entity = first_active_entity_ ? &entities_[first_active_entity_->index] : nullptr,
  };
}

Scene::EntityIterator Scene::end() {
  return {
    .scene = this,
    .entity = last_active_entity_ ? &entities_[last_active_entity_->index + 1] : nullptr,
  };
}

ComponentStorage* Scene::GetComponentStorage(TypeId component_type) {
  for (auto& storage : component_storages_) {
    if (storage.component_type_id() == component_type) {
      return &storage;
    }
  }
  return nullptr;
}

EventStorage* Scene::GetEventStorage(TypeId event_type) {
  for (auto& storage : event_storages_) {
    if (storage.event_type_id() == event_type) {
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

Result<> Scene::Prepare() {
  LogV("Preparing scene");
  {
    const auto scene_component_types = frame_scheduler().GetUsedSceneComponents();
    scene_components_.clear();
    scene_components_.reserve(scene_component_types.size());

    LogV(" Used Scene components:");
    for (const auto component_type : scene_component_types) {
      LogV(" - {}", main_vm->GetType(component_type)->GetReferenceString());
      scene_components_.insert(std::make_pair(component_type, Value(main_vm, component_type)));
    }
  }
  {
    const auto object_component_types = frame_scheduler().GetUsedEntityComponents();
    component_storages_.clear();
    component_storages_.reserve(object_component_types.size());

    LogV(" Used entity components:");
    for (const auto component_type : object_component_types) {
      LogV(" - {}", main_vm->GetType(component_type)->GetReferenceString());
      component_storages_.emplace_back(this, component_type, entities_.size());
    }
  }
  {
    const auto event_types = frame_scheduler().GetUsedEvents();
    event_storages_.clear();
    event_storages_.reserve(event_types.size());

    LogV(" Used events:");
    for (const auto event_type : event_types) {
      event_storages_.emplace_back(event_type);
      LogV(" - {}", main_vm->GetType(event_type)->GetReferenceString());
    }
  }
  return frame_scheduler_.Prepare(this);
}

void Scene::Play() {
  is_playing_ = true;
}

void Scene::Stop() {
  is_playing_ = false;
}

void Scene::Update(float delta_time) {
  assert(is_playing() && "Call Play() before calling Update().");
  for (auto& event_storage : event_storages_) {
    event_storage.Clear();
  }
  frame_scheduler_(SceneUpdate{.scene = this, .delta_time = delta_time});
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

EntityId Scene::InsertSibling(EntityId first_sibling_id, EntityId new_sibling_id) {
  Entity* first_sibling = GetEntityUnchecked(first_sibling_id);
  EntityId last_sibling_id = first_sibling->previous_sibling_id;

  Entity* previous_sibling;
  Entity* next_sibling;
  EntityId new_first_sibling_id;

  if (new_sibling_id.index < first_sibling_id.index) {
    new_first_sibling_id = new_sibling_id;
    previous_sibling = GetEntityUnchecked(last_sibling_id);
    next_sibling = first_sibling;
  } else {
    new_first_sibling_id = first_sibling_id;
    previous_sibling = first_sibling;
    next_sibling = GetEntityUnchecked(first_sibling->next_sibling_id);

    while (new_sibling_id.index < next_sibling->id.index && next_sibling->id.index > previous_sibling->id.index) {
      previous_sibling = next_sibling;
      next_sibling = GetEntityUnchecked(previous_sibling->next_sibling_id);
    }
  }

  Entity* new_sibling = GetEntityUnchecked(new_sibling_id);

  previous_sibling->next_sibling_id = new_sibling_id;
  new_sibling->previous_sibling_id = previous_sibling->id;
  new_sibling->next_sibling_id = next_sibling->id;
  next_sibling->previous_sibling_id = new_sibling_id;

  return new_first_sibling_id;
}

bool Scene::RemoveSibling(EntityId old_sibling_id) {
  if (GetEntityUnchecked(old_sibling_id)->next_sibling_id == old_sibling_id) {
    return false;
  } else {
    Entity* old_sibling = GetEntityUnchecked(old_sibling_id);
    Entity* next_sibling = GetEntityUnchecked(old_sibling->next_sibling_id);
    Entity* previous_sibling = GetEntityUnchecked(old_sibling->previous_sibling_id);
    next_sibling->previous_sibling_id = previous_sibling->id;
    previous_sibling->next_sibling_id = next_sibling->id;
    return true;
  }
}

}  // namespace ovis
