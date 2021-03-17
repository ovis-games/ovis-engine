#include <algorithm>

#include <SDL2/SDL_assert.h>

#include <ovis/core/log.hpp>
#include <ovis/scene/scene.hpp>
#include <ovis/scene/scene_controller.hpp>

namespace ovis {

SceneController::SceneController(const std::string& name)
    : name_(name)
#if OVIS_ENABLE_BUILT_IN_PROFILING
      ,
      update_profiler_(name + "Update")
#endif
{
}

SceneController::~SceneController() {}

void SceneController::Remove() {
  SDL_assert(scene_ != nullptr);
  scene()->RemoveController(name());
}

bool SceneController::IsSubscribedToEvent(const std::string& event_type) const {
  return subscribed_events_.count(event_type) > 0;
}

void SceneController::SubscribeToEvent(std::string event_type) {
  subscribed_events_.insert(std::move(event_type));
}

void SceneController::UnsubscribeFromEvent(const std::string& event_type) {
  subscribed_events_.erase(event_type);
}

void SceneController::Update(std::chrono::microseconds /*delta_time*/) {}

void SceneController::ProcessEvent(Event* event) {
  SDL_assert(IsSubscribedToEvent(std::string(event->type())));
  (void)event;
}

void SceneController::RegisterType(sol::table* module) {
  /// The base class for all scene controllers.
  // @classmod ovis.scene.SceneController
  sol::usertype<SceneController> scene_controller_type = module->new_usertype<SceneController>("SceneController");

  /// The scene, the controller is attached to.
  // @field[type=Scene] scene
  scene_controller_type["scene"] = sol::property(&SceneController::scene);
  
  /// The name of the controller.
  // @field[type=string] name
  scene_controller_type["name"] = sol::property(&SceneController::name);
}

void SceneController::UpdateBefore(const std::string& controller_name) {
  update_before_list_.insert(controller_name);
}

void SceneController::UpdateAfter(const std::string& controller_name) {
  update_after_list_.insert(controller_name);
}

}  // namespace ovis
