#pragma once

#include <ovis/core/scene_controller.hpp>
#include <ovis/core/scripting.hpp>

namespace ovis {

class VisualScriptSceneController : public SceneController {
 public:
  VisualScriptSceneController(std::string_view asset_id);
  ~VisualScriptSceneController() override;

  void Play() override;
  void Stop() override;
  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

  static void RegisterType(sol::table* module);

 private:
  std::optional<ScriptChunk> update_;
};

std::unique_ptr<VisualScriptSceneController> LoadVisualScriptSceneController(
    const std::string& asset_id, ScriptContext* script_context = global_script_context());

}  // namespace ovis
