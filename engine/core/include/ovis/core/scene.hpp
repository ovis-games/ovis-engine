#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_assert.h>
#include <sol/sol.hpp>

#include <ovis/utils/down_cast.hpp>
#include <ovis/utils/json.hpp>
#include <ovis/utils/range.hpp>
#include <ovis/utils/safe_pointer.hpp>
#include <ovis/utils/serialize.hpp>
#include <ovis/core/event.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/vector.hpp>

namespace ovis {

class SceneController;
class SceneObject;
class GraphicsContext;
class Window;
class SceneViewport;

class Scene : public Serializable, public SafelyReferenceable {
  friend class SceneController;
  friend class SceneObject;

 public:
  Scene();
  virtual ~Scene();

  inline bool is_playing() const { return is_playing_; }

  inline SceneViewport* main_viewport() const { return main_viewport_; }
  inline void SetMainViewport(SceneViewport* viewport) { main_viewport_ = viewport; }

  template <typename ControllerType, typename... ArgumentTypes>
  ControllerType* AddController(ArgumentTypes&&... constructor_args) {
    static_assert(std::is_base_of<SceneController, ControllerType>());
    return static_cast<ControllerType*>(
        AddController(std::make_unique<ControllerType>(std::forward<ArgumentTypes>(constructor_args)...)));
  }
  SceneController* AddController(std::unique_ptr<SceneController> scene_controller);
  SceneController* AddController(const std::string& id);
  SceneController* AddController(const std::string& id, const json& serialized_controller);
  // SceneController* AddController(const std::string& id, const sol::table& properties);
  void RemoveController(const std::string& id);
  void ClearControllers();
  bool HasController(const std::string& id) const;
  inline auto controller_ids() const { return Keys(controllers_); }
  inline auto controllers() const { return Values(controllers_); }

  template <typename ControllerType = SceneController>
  inline ControllerType* GetController(const std::string& controller_name) const {
    static_assert(std::is_base_of<SceneController, ControllerType>::value);
    return down_cast<ControllerType*>(GetControllerInternal(controller_name));
  }

  template <typename ControllerType>
  inline ControllerType* GetController() const {
    static_assert(std::is_base_of<SceneController, ControllerType>::value);
    return down_cast<ControllerType*>(GetControllerInternal(ControllerType::Name()));
  }

  SceneObject* CreateObject(const std::string& object_name);
  SceneObject* CreateObject(const std::string& object_name, const json& serialized_object);
  SceneObject* CreateObject(const std::string& object_name, const sol::table& properties);
  void DeleteObject(const std::string& object_name);
  void DeleteObject(SceneObject* object);
  void ClearObjects();
  SceneObject* GetObject(const std::string& object_name);
  bool ContainsObject(const std::string& object_name);

  void GetObjects(std::vector<SceneObject*>* scene_objects, bool sort_by_name = false) const;
  inline std::vector<SceneObject*> GetObjects(bool sort_by_name = false) const {
    std::vector<SceneObject*> objects;
    GetObjects(&objects, sort_by_name);
    return objects;
  }

  // TODO: create proper iterator here
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

  json Serialize() const override;
  bool Deserialize(const json& serialized_object) override;
  const json* GetSchema() const override { return &schema_; }

  static void RegisterType(sol::table* module);

 private:
  void InvalidateControllerOrder();
  void DeleteRemovedControllers();
  void SortControllers();
  SceneController* GetControllerInternal(std::string_view controller_name) const;

  std::map<std::string, std::unique_ptr<SceneController>, std::less<>> controllers_;
  std::vector<SceneController*> controller_order_;
  std::vector<std::unique_ptr<SceneController>> removed_controllers_;
  bool controllers_sorted_ = false;

  std::unordered_map<std::string, std::unique_ptr<SceneObject>> objects_;
  bool is_playing_ = false;

  SceneViewport* main_viewport_;

  static const json schema_;
};

}  // namespace ovis
