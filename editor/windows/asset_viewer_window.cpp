#include "asset_viewer_window.hpp"

#include <filesystem>
#include <fstream>

#include <imgui.h>

#include "asset_editors/scene_editor.hpp"
#include "asset_editors/script_editor.hpp"

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>

#include <ovis/engine/lua.hpp>

namespace ove {

AssetViewerWindow::AssetViewerWindow(AssetEditors* open_editors) : open_editors_(open_editors), current_path_("/") {
  ovis::Lua::on_error.Subscribe([this](const std::string& error_message) {
    std::vector<LuaError> errors = ParseLuaErrorMessage(error_message);
    if (errors.size() > 0) {
      ScriptEditor* script_editor = ovis::down_cast<ScriptEditor*>(OpenAssetEditor(errors[0].asset_id));
      script_editor->SetErrors(errors);
    }
  });
}

void AssetViewerWindow::Draw() {
  if (ImGui::Begin("Assets")) {
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
        OpenAssetEditor(asset_id);
      }
    }
    ImGui::EndChild();
  }
  ImGui::End();
}

AssetEditor* AssetViewerWindow::OpenAssetEditor(const std::string& asset_id) {
  for (const auto& asset_editor : *open_editors_) {
    if (asset_editor->asset_id() == asset_id) {
      asset_editor->Focus();
      return asset_editor.get();
    }
  }

  const std::string asset_type = ovis::GetApplicationAssetLibrary()->GetAssetType(asset_id);
  if (asset_type == "scene") {
    open_editors_->push_back(std::make_unique<SceneEditor>(asset_id));
    return open_editors_->back().get();
  } else if (asset_type == "scene_controller") {
    open_editors_->push_back(std::make_unique<ScriptEditor>(asset_id));
    return open_editors_->back().get();
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
