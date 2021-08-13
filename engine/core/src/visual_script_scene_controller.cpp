#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/script_error_event.hpp>
#include <ovis/core/visual_script_scene_controller.hpp>

namespace ovis {

VisualScriptSceneController::VisualScriptSceneController(std::string_view asset_id) : SceneController(asset_id) {
  auto chunk_or_error =
      ScriptChunk::Load(json::parse(GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id, "json").value()));
  if (std::holds_alternative<ScriptError>(chunk_or_error)) {
    LogE("Failed to load script controller: {}", std::get<ScriptError>(chunk_or_error).message);
  } else {
    update_.emplace(std::move(std::get<ScriptChunk>(chunk_or_error)));
  }
}

VisualScriptSceneController::~VisualScriptSceneController() {}

void VisualScriptSceneController::Play() {}

void VisualScriptSceneController::Stop() {}

void VisualScriptSceneController::Update(std::chrono::microseconds delta_time) {
  const auto result = update_->Execute(delta_time.count() / 1000000.0, this->scene());
  if (result.error) {
    LogE("Error: {}", result.error->message);
    ScriptErrorEvent error_event(name(), *result.error);
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
