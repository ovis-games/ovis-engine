#include "scene_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"

#include <imgui_stdlib.h>
#include <ovis/base/transform_component.hpp>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/utils.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/scene_object.hpp>
#include <ovis/rendering2d/sprite_component.hpp>
#include <ovis/physics2d/physics2d_debug_layer.hpp>

namespace ovis {
namespace editor {

SceneEditor::SceneEditor(const std::string& scene_asset) : SceneViewEditor(scene_asset) {
  SetupJsonFile(game_scene()->Serialize());
}

void SceneEditor::Save() {
  SaveFile("json", GetCurrentJsonFileState().dump());
}

void SceneEditor::CreateNew(const std::string& asset_id) {
  GetApplicationAssetLibrary()->CreateAsset(asset_id, "scene", {std::make_pair("json", Scene().Serialize().dump())});
}

void SceneEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  SDL_assert(file_type == "json");
  SetSerializedScene(data);
}

}  // namespace editor
}  // namespace ovis