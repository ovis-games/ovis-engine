#include "asset_viewer_window.hpp"

#include "../editor_window.hpp"
#include "asset_editors/scene_editor.hpp"
#include "asset_editors/scene_object_editor.hpp"
#include "asset_editors/script_editor.hpp"
#include "asset_editors/settings_editor.hpp"
#include "asset_editors/texture_editor.hpp"
#include "asset_importers/asset_importer.hpp"
#include "asset_editors/script_library_editor.hpp"
#include "asset_editors/visual_script_editor.hpp"
#include "dockspace_window.hpp"
#include <filesystem>
#include <fstream>

#include <imgui.h>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/script_error_event.hpp>

namespace ovis {
namespace editor {

AssetViewerWindow::AssetViewerWindow() : ImGuiWindow("Assets"), current_path_("/") {
  UpdateAfter("Dockspace Window");
  UpdateBefore("Overlay");

  SubscribeToEvent(LuaErrorEvent::TYPE);
  SubscribeToEvent("ScriptErrorEvent");
}

void AssetViewerWindow::DrawContent() {
  ImGui::BeginChild("AssetView", ImVec2(0, 0), false);

  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::BeginMenu("Create")) {
      if (ImGui::Selectable("Scene")) {
        SceneEditor::CreateNew(GetNewAssetName("New Scene"));
      }
      if (ImGui::Selectable("Scene Controller")) {
        ScriptEditor::CreateNew(GetNewAssetName("New Scene Controller"));
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }

  std::vector<std::string> assets = GetApplicationAssetLibrary()->GetAssets();
  std::sort(assets.begin(), assets.end());

  for (const auto& asset_id : assets) {
    ImGui::Selectable(asset_id.c_str());

    if (ImGui::BeginDragDropSource()) {
      ImGui::Text("%s", asset_id.c_str());

      const std::string type = "asset<" + GetApplicationAssetLibrary()->GetAssetType(asset_id) + '>';
      SDL_assert(type.size() <= 32);
      ImGui::SetDragDropPayload(type.c_str(), asset_id.c_str(), asset_id.length());
      ImGui::EndDragDropSource();
    }

    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::Selectable("Delete")) {
        EditorWindow::instance()->ShowMessageBox(
            fmt::format("Delete {}?", asset_id),
            fmt::format("Are you sure you want to delete the asset {}? This action cannot be undone!", asset_id),
            {"Delete Permanently", "Cancel"},
            [asset_id, asset_viewer_window = safe_ptr(this)](std::string_view pressed_button) {
              if (pressed_button == "Delete Permanently" && asset_viewer_window != nullptr) {
                asset_viewer_window->CloseAssetEditor(asset_id);
                GetApplicationAssetLibrary()->DeleteAsset(asset_id);
              }
            });
      }
      ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
      OpenAssetEditor(asset_id);
    }
  }

  ImGui::EndChild();

  if (ImGui::BeginDragDropTarget()) {
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_object");
    if (payload) {
      SceneObject* dragged_object = *reinterpret_cast<SceneObject**>(payload->Data);
      const std::string asset_name = GetNewAssetName(std::string(dragged_object->name()));
      GetApplicationAssetLibrary()->CreateAsset(asset_name, "scene_object",
                                                {{"json", dragged_object->Serialize().dump()}});
    }
    ImGui::EndDragDropTarget();
  }
}

void AssetViewerWindow::ProcessEvent(Event* event) {
 if (event->type() == LuaErrorEvent::TYPE) {
    auto* lua_error_event = down_cast<LuaErrorEvent*>(event);
    std::vector<LuaError> errors = ParseLuaErrorMessage(lua_error_event->message());
    if (errors.size() > 0) {
      ScriptEditor* script_editor = down_cast<ScriptEditor*>(OpenAssetEditor(errors[0].asset_id));
      script_editor->SetErrors(errors);
    }
  } else if (event->type() == "ScriptErrorEvent") {
    auto* script_error_event = down_cast<ScriptErrorEvent*>(event);
    VisualScriptEditor* script_editor = down_cast<VisualScriptEditor*>(OpenAssetEditor(std::string(script_error_event->asset_id())));
    script_editor->SetError(script_error_event->error());
  }
}

AssetEditor* AssetViewerWindow::OpenAssetEditor(const std::string& asset_id) {
  auto editor = scene()->GetController<AssetEditor>(AssetEditor::GetAssetEditorId(asset_id));
  if (editor) {
    editor->Focus();
    return editor;
  }

  const std::string asset_type = GetApplicationAssetLibrary()->GetAssetType(asset_id);
  AssetEditor* asset_editor = nullptr;
  if (asset_type == "scene") {
    asset_editor = scene()->AddController<SceneEditor>(asset_id);
  } else if (asset_type == "scene_object") {
    asset_editor = scene()->AddController<SceneObjectEditor>(asset_id);
  } else if (asset_type == "scene_controller") {
    asset_editor = scene()->AddController<VisualScriptEditor>(asset_id);
  } else if (asset_type == "texture2d") {
    asset_editor = scene()->AddController<TextureEditor>(asset_id);
  } else if (asset_type == "settings") {
    asset_editor = scene()->AddController<SettingsEditor>(asset_id);
  } else if (asset_type == "script_library") {
    asset_editor = scene()->AddController<ScriptLibraryEditor>(asset_id);
  } else {
    LogE("Unknown asset type: {}", asset_type);
  }

  if (asset_editor != nullptr) {
    asset_editor->SetDockSpaceId(scene()->GetController<DockspaceWindow>("Dockspace Window")->dockspace_main());
  }
  return asset_editor;
}

bool AssetViewerWindow::CloseAssetEditor(const std::string& asset_id) {
  auto editor = scene()->GetController<AssetEditor>(AssetEditor::GetAssetEditorId(asset_id));
  if (editor) {
    editor->Remove();
    return true;
  } else {
    return true;
  }
}

std::string AssetViewerWindow::GetNewAssetName(const std::string& base_name) const {
  std::string asset_name = base_name;
  for (int i = 0; GetApplicationAssetLibrary()->Contains(asset_name); asset_name = base_name + std::to_string(++i))
    ;
  return asset_name;
}

}  // namespace editor
}  // namespace ovis
