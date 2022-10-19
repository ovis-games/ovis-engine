#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ovis/core/scheduler.hpp"
#include "ovis/utils/all.hpp"
#include "ovis/utils/down_cast.hpp"
#include "ovis/utils/json.hpp"
#include "ovis/utils/range.hpp"
#include "ovis/utils/safe_pointer.hpp"
#include "ovis/utils/serialize.hpp"
#include "ovis/core/component_storage.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/event.hpp"
#include "ovis/core/vector.hpp"

namespace ovis {

class Scene;

struct SceneUpdate {
  Scene* scene;
  float delta_time;
};

class Scene : public Serializable {
  friend class SceneController;
  friend class SceneObject;

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
  auto GetEntityIds() const {
    return TransformRange(FilterRange(entities_, [](const auto& obj) { return obj.is_active(); }),
                          [](const auto& obj) { return obj.id; });
  }

  template <typename ComponentType>
  ComponentStorage* GetComponentStorage() {
    return GetComponentStorage(main_vm->GetTypeId<ComponentType>());
  }
  ComponentStorage* GetComponentStorage(TypeId component_type);

  void Prepare();

  void Play();
  void Stop();

  void Update(float delta_time);

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;

 private:
  Scheduler<Scene*, SceneUpdate> frame_scheduler_;

  std::vector<Entity> entities_;
  std::optional<EntityId> first_active_entity_;
  std::optional<EntityId> first_inactive_entity_;

  std::vector<ComponentStorage> component_storages_;

  bool is_playing_ = false;

  // Inserts a sibling in an existing sibling chain. All sibling indices in the chain as well as
  // the new entity are corrected. It returns the id of the first sibling after the insertion.
  [[nodiscard]] EntityId InsertSibling(EntityId first_sibling_id, EntityId new_sibling_id);
  bool RemoveSibling(EntityId old_sibling_id);
};

}  // namespace ovis
