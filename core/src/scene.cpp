#include "ovis/core/scene.hpp"

#include <iterator>
#include <map>
#include <string>

#include "ovis/utils/log.hpp"
#include "ovis/utils/utf8.hpp"
#include "ovis/core/asset_library.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/scene_viewport.hpp"
#include "schemas/scene.hpp"

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

Entity* Scene::CreateEntity(std::string_view object_name, const json& serialized_entity, std::optional<EntityId> parent) {
  Entity* entity = CreateEntity(object_name, parent);

  schemas::EntitySchema definition = serialized_entity;
  for (const auto& [component_id, component_definition] : definition.components) {
    auto storage = GetComponentStorage(main_vm->GetTypeId(component_id));
    if (storage) {
      storage->AddComponent(entity->id);
    }
  }
  for (const auto& [child_name, child_definition] : definition.children) {
    CreateEntity(child_name, child_definition, entity->id);
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

void Scene::ClearEntities() {
  for (auto& entity_component_storage : component_storages_) {
    entity_component_storage.Clear();
  }
  for (auto& entity : entities_) {
    if (entity.is_active()) {
      entity.id = entity.id.next();
      entity.parent_id = entity.id;
      entity.first_children_id = entity.id;
      entity.deactivate();
    }
  }
  for (auto& entity : entities_) {
    entity.previous_sibling_id = entities_[entity.id.index > 0 ? entity.id.index - 1 : entities_.size() -1].id;
    entity.next_sibling_id = entities_[(entity.id.index + 1) % entities_.size()].id;
  }
  first_inactive_entity_.emplace(entities_[0].id);
  first_active_entity_.reset();
  last_active_entity_.reset();
}

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
  schemas::Scene scene;
  return scene;
}

bool Scene::Deserialize(const json& serialized_scene) {
  schemas::Scene scene = serialized_scene;

  ClearEntities();

  for (const auto& [entity_name, entity_definition]: scene.entities) {
    this->CreateEntity(entity_name, entity_definition);
  }

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
