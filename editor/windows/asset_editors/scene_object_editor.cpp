#include "scene_object_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"

#include <imgui_stdlib.h>

#include <ovis/core/asset_library.hpp>

namespace ovis {
namespace editor {

SceneObjectEditor::SceneObjectEditor(const std::string& scene_object_asset) : SceneViewEditor(scene_object_asset, false) {
  object_ = game_scene()->CreateObject(scene_object_asset);
  SelectObject(object_.get());
  SetupJsonFile(object_->Serialize());
}

void SceneObjectEditor::Save() {
  SaveFile("json", object_->Serialize().dump());
}

void SceneObjectEditor::SubmitChanges() {
  UpdateSceneEditingCopy();
  SubmitJsonFile(object_->Serialize());
}

void SceneObjectEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  SDL_assert(file_type == "json");
  object_->Deserialize(data);
  UpdateSceneEditingCopy();
}

void SceneObjectEditor::DrawObjectTree() {
  DrawObjectHierarchy(object_.get());
}

}  // namespace editor
}  // namespace ovis
