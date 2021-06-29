#include "scene_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"
#include "editing_controllers/object_selection_controller.hpp"

#include <imgui_stdlib.h>

#include <ovis/core/asset_library.hpp>

namespace ovis {
namespace editor {

SceneEditor::SceneEditor(const std::string& scene_asset) : SceneViewEditor(scene_asset, true) {
  SetupJsonFile(game_scene()->Serialize());
}

void SceneEditor::Save() {
  SaveFile("json", GetCurrentJsonFileState().dump());
}

void SceneEditor::CreateNew(const std::string& asset_id) {
  GetApplicationAssetLibrary()->CreateAsset(asset_id, "scene", {std::make_pair("json", Scene().Serialize().dump())});
}

void SceneEditor::SubmitChanges() {
  UpdateSceneEditingCopy();
  if (run_state() == RunState::STOPPED) {
    SubmitJsonFile(game_scene()->Serialize());
  }
}

void SceneEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  SDL_assert(file_type == "json");
  ChangeRunState(RunState::STOPPED);
  game_scene()->Deserialize(data);
  UpdateSceneEditingCopy();
}

void SceneEditor::DrawObjectTree() {
  ImGuiIO& io = ImGui::GetIO();
  const bool control_or_command = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();

  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::Selectable("Create Object")) {
      object_selection_controller->SelectObject(game_scene()->CreateObject("New Object"));
      SubmitChanges();
      RenameSelectedObject();
    }
    ImGui::EndPopup();
  }

  if (ImGui::IsWindowFocused() && control_or_command && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V))) {
    PasteObjectFromClipboard();
  }

  ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                       ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
  ImGuiTreeNodeFlags scene_node_flags = tree_node_flags | ImGuiTreeNodeFlags_AllowItemOverlap;
  if (!object_selection_controller->has_selected_object()) {
    // If there is no selected object, the scene is selected
    scene_node_flags |= ImGuiTreeNodeFlags_Selected;
  }
  if (ImGui::TreeNodeEx(asset_id().c_str(), scene_node_flags)) {
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::Selectable("Create Object")) {
        object_selection_controller->SelectObject(game_scene()->CreateObject("New Object"));
        SubmitChanges();
        RenameSelectedObject();
      }
      ImGui::EndPopup();
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      object_selection_controller->ClearSelection();
    }
    if (ImGui::BeginDragDropTarget()) {
      const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_object");
      if (payload) {
        SceneObject* dragged_object = *reinterpret_cast<SceneObject**>(payload->Data);
        if (dragged_object->scene() != game_scene() || dragged_object->parent() != nullptr) {
          DoOnceAfterUpdate([this, dragged_object]() {
            game_scene()->CreateObject(dragged_object->name(), dragged_object->Serialize());
            dragged_object->scene()->DeleteObject(dragged_object);
            SubmitChanges();
          });
        }
      }
      ImGui::EndDragDropTarget();
    }

    for (SceneObject* object : game_scene()->root_objects()) {
      SDL_assert(object != nullptr);
      SDL_assert(game_scene()->ContainsObject(object->name()));
      DrawObjectHierarchy(object);
    }

    ImGui::TreePop();
  }
}

}  // namespace editor
}  // namespace ovis
