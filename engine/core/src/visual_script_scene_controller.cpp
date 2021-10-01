#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/script_error.hpp>
#include <ovis/core/visual_script_scene_controller.hpp>

namespace ovis {

VisualScriptSceneController::VisualScriptSceneController(std::string_view asset_id) : SceneController(asset_id) {
  auto error = global_script_context()->LoadAsset(asset_id);
  if (error.has_value()) {
    LogE("Failed to load script controller: {}", error->message);
  }
}

VisualScriptSceneController::~VisualScriptSceneController() {}

void VisualScriptSceneController::Play() {}

void VisualScriptSceneController::Stop() {}

void VisualScriptSceneController::Update(std::chrono::microseconds delta_time) {
  const auto result = global_script_context()->Call(fmt::format("{}.update", name()), delta_time.count() / 1000000.0, this->scene());
  if (std::holds_alternative<ScriptError>(result)) {
    ScriptErrorEvent error_event(name(), std::get<ScriptError>(result));
    PostGlobalEvent(&error_event);
  }
}

void VisualScriptSceneController::ProcessEvent(Event* event) {}

std::unique_ptr<VisualScriptSceneController> LoadVisualScriptSceneController(const std::string& asset_id,
                                                                             ScriptContext* context) {
  if (GetApplicationAssetLibrary() != nullptr && GetApplicationAssetLibrary()->Contains(asset_id) &&
      GetApplicationAssetLibrary()->GetAssetType(asset_id) == "scene_controller") {
    return std::make_unique<VisualScriptSceneController>(asset_id);
  }
  return nullptr;
}

}  // namespace ovis
