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
  Scene(std::size_t initial_object_capacity = 1000);
  virtual ~Scene();

  inline bool is_playing() const { return is_playing_; }

  auto& frame_scheduler() { return frame_scheduler_; }

  Entity* CreateEntity(std::string_view object_name, std::optional<Entity::Id> parent = std::nullopt);
  Entity* CreateEntity(std::string_view object_name, const json& serialized_object, std::optional<Entity::Id> parent = std::nullopt);

  Entity* GetEntity(Entity::Id id);
  Entity* GetEntity(std::string_view object_path) const;
  Entity::Id GetEntityId(std::string_view object_path) const { return GetEntity(object_path)->id(); }

  void DeleteEntity(std::string_view object_path) { return DeleteEntity(GetEntityId(object_path)); }
  void DeleteEntity(Entity* object) { return DeleteEntity(object->id()); }
  void DeleteEntity(Entity::Id id);
  void ClearEntities();

  bool IsEntityIdValid(Entity::Id id) { return id.index < entities_.size() && entities_[id.index].id() == id; }
  auto GetEntityIds() const {
    return TransformRange(FilterRange(entities_, [](const auto& obj) { return obj.is_alive(); }),
                          [](const auto& obj) { return obj.id(); });
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
  std::vector<ComponentStorage> component_storages_;

  bool is_playing_ = false;
};

}  // namespace ovis
