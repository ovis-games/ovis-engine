#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_assert.h>
#include <sol/sol.hpp>

#include "ovis/utils/all.hpp"
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
  std::string CreateControllerName(std::string_view base_name);
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

  SceneObject* CreateObject(std::string_view object_name, SceneObject* parent = nullptr);
  SceneObject* CreateObject(std::string_view object_name, const json& serialized_object, SceneObject* parent = nullptr);
  void DeleteObject(std::string_view object_path);
  void DeleteObject(SceneObject* object);
  void ClearObjects();
  SceneObject* GetObject(std::string_view object_reference);
  bool ContainsObject(std::string_view object_reference);
  inline auto objects() const {
    return TransformRange(objects_, [](const auto& name_object) { return name_object.second.get(); });
  }
  inline auto root_objects() const {
    return FilterRange(objects(), [](const SceneObject* object) { return !object->has_parent(); });
  }

  template <typename ComponentType>
  auto ObjectComponentsOfType() {
    return FilterRange(
        TransformRange(objects_, [](auto& object) { return object.second->template GetComponent<ComponentType>(); }),
        [](ComponentType* component) { return component != nullptr; });
  }
  template <typename ComponentType>
  auto ObjectsWithComponent() {
    return FilterRange(
        TransformRange(objects_, [](auto& object) { return object.second.get(); }),
        [](SceneObject* object) { return object != nullptr; });
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
  std::size_t event_handler_index_;

  SceneViewport* main_viewport_ = nullptr;

  static const json schema_;
};

}  // namespace ovis
