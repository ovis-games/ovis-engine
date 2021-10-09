#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/script_error.hpp>
#include <ovis/core/visual_script_scene_controller.hpp>

namespace ovis {

VisualScriptSceneController::VisualScriptSceneController(std::string_view asset_id) : SceneController(asset_id) {
  AssetLibrary* asset_library = GetAssetLibraryForAsset(asset_id);
  {
    const auto serialized_update_function = asset_library->LoadAssetTextFile(asset_id, "Update.function.json");
    if (serialized_update_function.has_value()) {
      const auto chunk = ScriptChunk::Load(global_script_context(), json::parse(*serialized_update_function));
      if (std::holds_alternative<ScriptError>(chunk)) {
        LogE("Failed to load Update function of script controller: {}", std::get<ScriptError>(chunk).message);
      } else {
        update_chunk_.emplace(std::get<ScriptChunk>(chunk));
        LogD("Update function of {} loaded", asset_id);
      }
    }
  }
  {
    const auto serialized_play_function = asset_library->LoadAssetTextFile(asset_id, "Play.function.json");
    if (serialized_play_function.has_value()) {
      const auto chunk = ScriptChunk::Load(global_script_context(), json::parse(*serialized_play_function));
      if (std::holds_alternative<ScriptError>(chunk)) {
        LogE("Failed to load Play function of script controller: {}", std::get<ScriptError>(chunk).message);
      } else {
        play_chunk_.emplace(std::get<ScriptChunk>(chunk));
        LogD("Play function of {} loaded", asset_id);
      }
    }
  }
  {
    const auto serialized_stop_function = asset_library->LoadAssetTextFile(asset_id, "Stop.function.json");
    if (serialized_stop_function.has_value()) {
      const auto chunk = ScriptChunk::Load(global_script_context(), json::parse(*serialized_stop_function));
      if (std::holds_alternative<ScriptError>(chunk)) {
        LogE("Failed to load Update function of script controller: {}", std::get<ScriptError>(chunk).message);
      } else {
        stop_chunk_.emplace(std::get<ScriptChunk>(chunk));
        LogD("Stop function of {} loaded", asset_id);
      }
    }
  }
  // auto error = global_script_context()->LoadAsset(asset_id);
  // if (error.has_value()) {
  //   LogE("Failed to load script controller: {}", error->message);
  // }
}

VisualScriptSceneController::~VisualScriptSceneController() {}

void VisualScriptSceneController::Play() {
  if (play_chunk_.has_value()) {
    const auto result = play_chunk_->Call(scene());
    if (std::holds_alternative<ScriptError>(result)) {
      ScriptErrorEvent error_event(name(), std::get<ScriptError>(result));
      PostGlobalEvent(&error_event);
    }
  }
}

void VisualScriptSceneController::Stop() {
  if (stop_chunk_.has_value()) {
    const auto result = stop_chunk_->Call(scene());
    if (std::holds_alternative<ScriptError>(result)) {
      ScriptErrorEvent error_event(name(), std::get<ScriptError>(result));
      PostGlobalEvent(&error_event);
    }
  }
}

void VisualScriptSceneController::Update(std::chrono::microseconds delta_time) {
  if (update_chunk_.has_value()) {
    const auto result = update_chunk_->Call(scene(), delta_time.count() / 1000000.0);
    if (std::holds_alternative<ScriptError>(result)) {
      ScriptErrorEvent error_event(name(), std::get<ScriptError>(result));
      PostGlobalEvent(&error_event);
    }
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
