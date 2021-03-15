#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_assert.h>
#include <sol/sol.hpp>

#include <ovis/core/down_cast.hpp>
#include <ovis/core/json.hpp>
#include <ovis/core/serialize.hpp>
#include <ovis/math/vector.hpp>
#include <ovis/engine/camera.hpp>
#include <ovis/engine/event.hpp>
#include <ovis/engine/scene_object.hpp>

namespace ovis {

class SceneController;
class SceneObject;
class GraphicsContext;
class ResourceManager;
class Window;

class Scene : public Serializable {
  friend class SceneController;
  friend class SceneObject;

 public:
  Scene();
  virtual ~Scene();

  inline bool is_playing() const { return is_playing_; }

  inline ResourceManager* resource_manager() const { return resource_manager_; }
  inline void SetResourceManager(ResourceManager* resource_manager) { resource_manager_ = resource_manager; }

  template <typename ControllerType, typename... ArgumentTypes>
  ControllerType* AddController(ArgumentTypes&&... constructor_args) {
    static_assert(std::is_base_of<SceneController, ControllerType>());
    return static_cast<ControllerType*>(
        AddController(std::make_unique<ControllerType>(std::forward<ArgumentTypes>(constructor_args)...)));
  }
  SceneController* AddController(std::unique_ptr<SceneController> scene_controller);
  SceneController* AddController(const std::string& id);
  void RemoveController(const std::string& id);
  void ClearControllers();

  template <typename ControllerType = SceneController>
  inline ControllerType* GetController(const std::string& controller_name) const {
    static_assert(std::is_base_of<SceneController, ControllerType>::value, "");
    return down_cast<ControllerType*>(GetControllerInternal(controller_name));
  }

  SceneObject* CreateObject(const std::string& object_name);
  SceneObject* CreateObject(const std::string& object_name, const json& serialized_object);
  void DeleteObject(const std::string& object_name);
  void ClearObjects();
  SceneObject* GetObject(const std::string& object_name);
  bool ContainsObject(const std::string& object_name);

  void GetObjects(std::vector<SceneObject*>* scene_objects, bool sort_by_name = false) const;
  inline std::vector<SceneObject*> GetObjects(bool sort_by_name = false) const {
    std::vector<SceneObject*> objects;
    GetObjects(&objects, sort_by_name);
    return objects;
  }

  void GetSceneObjectsWithComponent(const std::string& component_id, std::vector<SceneObject*>* scene_objects) const;

  inline std::vector<SceneObject*> GetSceneObjectsWithComponent(const std::string& component_id) const {
    std::vector<SceneObject*> scene_objects;
    GetSceneObjectsWithComponent(component_id, &scene_objects);
    return scene_objects;
  }

  void Play();
  void Stop();

  void BeforeUpdate();
  void AfterUpdate();
  void Update(std::chrono::microseconds delta_time);

  void ProcessEvent(Event* event);

  void DrawImGui();

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;
  const json* GetSchema() const override { return &schema_; }

  static void RegisterType(sol::table* module);

 private:
  void InvalidateControllerOrder();
  void DeleteRemovedControllers();
  void SortControllers();
  SceneController* GetControllerInternal(const std::string& controller_name) const;

  std::unordered_map<std::string, std::unique_ptr<SceneController>> controllers_;
  std::vector<SceneController*> controller_order_;
  std::vector<std::unique_ptr<SceneController>> removed_controllers_;
  bool controllers_sorted_ = false;

  std::unordered_map<std::string, std::unique_ptr<SceneObject>> objects_;
  ResourceManager* resource_manager_;
  bool is_playing_ = false;

  static const json schema_;
};

}  // namespace ovis
