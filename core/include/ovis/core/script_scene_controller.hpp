#pragma once

#include <sol/sol.hpp>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class ScriptSceneController : public SceneController {
 public:
  ScriptSceneController(const std::string& name, sol::table class_table);
  ~ScriptSceneController() override;

  inline bool is_valid() { return instance_ && instance_.get_type() == sol::type::table; }

  void Play() override;
  void Stop() override;
  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

  static void RegisterType(sol::table* module);

 private:
  sol::table instance_;
  sol::protected_function play_function_;
  sol::protected_function stop_function_;
  sol::protected_function update_function_;
  sol::protected_function process_event_function_;
};

std::unique_ptr<ScriptSceneController> LoadScriptSceneController(const std::string& asset_id, sol::state* lua_state);

}  // namespace ovis
