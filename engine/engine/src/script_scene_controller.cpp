#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/script_scene_controller.hpp>

namespace ovis {

ScriptSceneController::ScriptSceneController(const std::string& name, sol::table class_table) : SceneController(name) {
  sol::table object = class_table;
  sol::protected_function new_function = object["new"];
  SDL_assert(new_function != sol::lua_nil);
  instance_ = new_function.call(object);
  SDL_assert(instance_ != sol::lua_nil);
  update_function_ = instance_["Update"];
  SDL_assert(update_function_ != sol::lua_nil);
}

void ScriptSceneController::Play() {
  instance_["scene"] = scene();

  sol::protected_function play_function = instance_["Play"];
  if (play_function != sol::lua_nil) {
    sol::protected_function_result pfr = play_function.call(instance_);
  }
}

void ScriptSceneController::Stop() {
  instance_["scene"] = scene();
  if (instance_["Stop"] != sol::lua_nil) {
    instance_["Stop"](instance_);
  }
}

void ScriptSceneController::Update(std::chrono::microseconds delta_time) {
  instance_["scene"] = scene();
  SDL_assert(update_function_ != sol::lua_nil);
  sol::protected_function_result pfr = update_function_.call(instance_, delta_time.count() / 1000000.0);
}

}  // namespace ovis