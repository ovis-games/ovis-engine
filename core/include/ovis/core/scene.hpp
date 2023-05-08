#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ovis/utils/all.hpp"
#include "ovis/utils/down_cast.hpp"
#include "ovis/utils/json.hpp"
#include "ovis/utils/range.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/utils/safe_pointer.hpp"
#include "ovis/utils/serialize.hpp"
#include "ovis/core/component_storage.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/event_storage.hpp"
#include "ovis/core/job.hpp"
#include "ovis/core/scheduler.hpp"
#include "ovis/core/vector.hpp"

namespace ovis {

class Scene;

struct SceneUpdate {
  Scene* scene;
  float delta_time;
};

using FrameScheduler = Scheduler<Scene*, SceneUpdate>;
using FrameJob = Job<Scene*, SceneUpdate>;

class Scene : public Serializable {
  friend class SceneController;

 public:
  Scene(std::size_t initial_entity_capacity = 1000);
  virtual ~Scene();

  inline bool is_playing() const { return is_playing_; }

  auto& frame_scheduler() { return frame_scheduler_; }

  Entity* CreateEntity(std::string_view object_name, std::optional<EntityId> parent = std::nullopt);
  Entity* CreateEntity(std::string_view object_name, const json& serialized_object, std::optional<EntityId> parent = std::nullopt);

  Entity* GetEntity(EntityId id);
  Entity* GetEntityUnchecked(EntityId id);
  Entity* GetEntity(std::string_view entity_path) const;
  EntityId GetEntityId(std::string_view entity_path) const { return GetEntity(entity_path)->id; }

  void DeleteEntity(std::string_view entity_path) { return DeleteEntity(GetEntityId(entity_path)); }
  void DeleteEntity(Entity* entity) { return DeleteEntity(entity->id); }
  void DeleteEntity(EntityId id);
  void ClearEntities();

  bool IsEntityIdValid(EntityId id) { return id.index < entities_.size() && entities_[id.index].id == id; }

  struct EntityIterator;
  friend struct EntityIterator;
  EntityIterator begin();
  EntityIterator end();

  Range<Entity::AscendingSiblingIterator> root_entities() {
    return {
      Entity::AscendingSiblingIterator {
        .scene = this,
        .entity = first_active_entity_ ? GetEntityUnchecked(*first_active_entity_) : nullptr,
      },
      Entity::AscendingSiblingIterator {
        .scene = this,
        .entity = nullptr,
      },
    };
  }

  template <typename ComponentType>
  ComponentStorageView<ComponentType> GetComponentStorage() {
    return GetComponentStorage(main_vm->GetTypeId<ComponentType>());
  }
  ComponentStorage* GetComponentStorage(TypeId component_type);

  template <typename EventType>
  EventEmitter<EventType> GetEventEmitter() {
    return GetEventStorage(main_vm->GetTypeId<EventType>());
  }
  template <typename EventType>
  EventStorageView<EventType> GetEventStorage() {
    return GetEventStorage(main_vm->GetTypeId<EventType>());
  }
  EventStorage* GetEventStorage(TypeId event_type);

  Result<> Prepare();

  void Play();
  void Stop();

  void Update(float delta_time);

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;

 private:
  FrameScheduler frame_scheduler_;

  std::vector<Entity> entities_;
  std::optional<EntityId> first_active_entity_;
  std::optional<EntityId> last_active_entity_;
  std::optional<EntityId> first_inactive_entity_;

  std::vector<ComponentStorage> component_storages_;
  std::vector<EventStorage> event_storages_;

  bool is_playing_ = false;

  // Inserts a sibling in an existing sibling chain. All sibling indices in the chain as well as
  // the new entity are corrected. It returns the id of the first sibling after the insertion.
  [[nodiscard]] EntityId InsertSibling(EntityId first_sibling_id, EntityId new_sibling_id);
  bool RemoveSibling(EntityId old_sibling_id);
};

struct Scene::EntityIterator : ovis::EntityIterator {
  Entity& operator*() const { return *entity; }
  Entity* operator->() { return entity; }
  EntityIterator& operator++() {
    Increment();
    return *this;
  }
  EntityIterator operator++(int) {
    auto current = *this;
    Increment();
    return current;
  }

  Scene* scene;
  Entity* entity;

  void Increment() {
    assert(scene->last_active_entity_.has_value() &&
           "This has to be true, otherwise the scene would not contain any entities and an increment would be illegal");
    std::size_t entity_index = entity->id.index;
    do {
      ++entity_index;
    } while (entity_index <= scene->last_active_entity_->index && !scene->entities_[entity_index].is_active());

    entity = &scene->entities_[entity_index];
  }
};

inline bool operator==(const Scene::EntityIterator& lhs, const Scene::EntityIterator& rhs) {
  assert(lhs.scene == rhs.scene);
  return lhs.entity == rhs.entity;
}

inline bool operator!=(const Scene::EntityIterator& lhs, const Scene::EntityIterator& rhs) {
  assert(lhs.scene == rhs.scene);
  return lhs.entity != rhs.entity;
}

}  // namespace ovis
