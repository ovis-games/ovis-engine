#include "asset_viewer_window.hpp"

#include "asset_editors/scene_editor.hpp"
#include "asset_editors/script_editor.hpp"
#include "asset_editors/texture_editor.hpp"
#include "dockspace_window.hpp"
#include <filesystem>
#include <fstream>

#include <imgui.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>
#include <ovis/engine/lua.hpp>

namespace ove {

AssetViewerWindow::AssetViewerWindow() : UiWindow("Assets"), current_path_("/") {
  UpdateAfter("Dockspace Window");

  ovis::Lua::on_error.Subscribe([this](const std::string& error_message) {
    std::vector<LuaError> errors = ParseLuaErrorMessage(error_message);
    if (errors.size() > 0) {
      ScriptEditor* script_editor = ovis::down_cast<ScriptEditor*>(OpenAssetEditor(errors[0].asset_id));
      script_editor->SetErrors(errors);
    }
  });
}

void AssetViewerWindow::DrawContent() {
  ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "/");
  ImGui::SameLine();
  if (ImGui::BeginCombo("###AddAsset", "Add Asset", ImGuiComboFlags_NoArrowButton)) {
    if (ImGui::Selectable("Scene")) {
      // asset_library_.AddScene(GetNewAssetName("NewScene"));
    }
    if (ImGui::Selectable("Scene Controller")) {
      // asset_library_.AddSceneControllerScript(GetNewAssetName("NewSceneController"));
    }
    ImGui::EndCombo();
  }

  ImGui::BeginChild("AssetView", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);

  for (const auto& asset_id : ovis::GetApplicationAssetLibrary()->GetAssets()) {
    ImGui::Selectable(asset_id.c_str());

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
      if (auto asset_editor = OpenAssetEditor(asset_id); asset_editor != nullptr) {
        asset_editor->SetDockSpaceId(scene()->GetController<DockspaceWindow>("Dockspace Window")->dockspace_main());
      }
    }
  }
  ImGui::EndChild();
}

AssetEditor* AssetViewerWindow::OpenAssetEditor(const std::string& asset_id) {
  auto editor = scene()->GetController<AssetEditor>(AssetEditor::GetAssetEditorId(asset_id));
  if (editor) {
    editor->Focus();
    return editor;
  }

  const std::string asset_type = ovis::GetApplicationAssetLibrary()->GetAssetType(asset_id);
  if (asset_type == "scene") {
    return scene()->AddController<SceneEditor>(asset_id);
  } else if (asset_type == "scene_controller") {
    return scene()->AddController<ScriptEditor>(asset_id);
  } else if (asset_type == "texture2d") {
    return scene()->AddController<TextureEditor>(asset_id);
  } else {
    ovis::LogE("Unknown asset type: {}", asset_type);
    return nullptr;
  }
}

std::string AssetViewerWindow::GetNewAssetName(const std::string& base_name) const {
  std::string asset_name = base_name;
  for (int i = 0; ovis::GetApplicationAssetLibrary()->Contains(asset_name);
       asset_name = base_name + std::to_string(++i))
    ;
  return asset_name;
}

}  // namespace ove
