#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/visual_script_scene_controller.hpp>

namespace ovis {

VisualScriptSceneController::VisualScriptSceneController(std::string_view asset_id) : SceneController(asset_id) {
  update_.emplace(json::parse(GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id, "json").value()));
}

VisualScriptSceneController::~VisualScriptSceneController() {}

void VisualScriptSceneController::Play() {}

void VisualScriptSceneController::Stop() {}

void VisualScriptSceneController::Update(std::chrono::microseconds delta_time) {
  update_->Execute(delta_time.count() / 1000000.0, this->scene());
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