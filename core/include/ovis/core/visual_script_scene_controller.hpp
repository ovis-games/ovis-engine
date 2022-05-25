#pragma once

#include <ovis/core/scene_controller.hpp>

namespace ovis {

// class VisualScriptSceneController : public SceneController {
//  public:
//   VisualScriptSceneController(std::string_view asset_id);
//   ~VisualScriptSceneController() override;

//   void Play() override;
//   void Stop() override;
//   void Update(std::chrono::microseconds delta_time) override;
//   void ProcessEvent(Event* event) override;

//   static void RegisterType(sol::table* module);

//  private:
//   // std::optional<ScriptFunction> play_function_;
//   // std::optional<ScriptFunction> stop_function_;
//   // std::optional<ScriptFunction> update_function_;
// };

// std::unique_ptr<VisualScriptSceneController> LoadVisualScriptSceneController(const std::string& asset_id);

}  // namespace ovis
