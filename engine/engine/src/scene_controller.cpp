#include <algorithm>

#include <SDL2/SDL_assert.h>

#include <ovis/core/log.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>

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

void SceneController::RegisterToLua() {
  // auto& lua = Lua::state;
  // sol::usertype<SceneController> scene_controller_type = lua.new_usertype<SceneController>("SceneController");
}

std::vector<std::string> SceneController::GetRegisteredControllers() {
  std::vector<std::string> registered_controllers;
  registered_controllers.reserve(Module::scene_controller_factory_functions()->size());
  for (const auto& controller_factory : *Module::scene_controller_factory_functions()) {
    registered_controllers.push_back(controller_factory.first);
  }
  return registered_controllers;
}

void SceneController::UpdateBefore(const std::string& controller_name) {
  update_before_list_.insert(controller_name);
}

void SceneController::UpdateAfter(const std::string& controller_name) {
  update_after_list_.insert(controller_name);
}

}  // namespace ovis
