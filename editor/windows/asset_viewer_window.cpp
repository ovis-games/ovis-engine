#include "asset_viewer_window.hpp"

#include "asset_editors/scene_editor.hpp"
#include "asset_editors/script_editor.hpp"
#include "asset_editors/settings_editor.hpp"
#include "asset_editors/texture_editor.hpp"
#include "asset_importers/asset_importer.hpp"
#include "dockspace_window.hpp"
#include <filesystem>
#include <fstream>

#include <imgui.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>
#include <ovis/engine/lua.hpp>

namespace ovis {
namespace editor {

AssetViewerWindow::AssetViewerWindow() : UiWindow("Assets"), current_path_("/") {
  UpdateAfter("Dockspace Window");

  Lua::on_error.Subscribe([this](const std::string& error_message) {
    std::vector<LuaError> errors = ParseLuaErrorMessage(error_message);
    if (errors.size() > 0) {
      ScriptEditor* script_editor = down_cast<ScriptEditor*>(OpenAssetEditor(errors[0].asset_id));
      script_editor->SetErrors(errors);
    }
  });
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
        GetApplicationAssetLibrary()->DeleteAsset(asset_id);
      }
      ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
      if (auto asset_editor = OpenAssetEditor(asset_id); asset_editor != nullptr) {
        asset_editor->SetDockSpaceId(scene()->GetController<DockspaceWindow>("Dockspace Window")->dockspace_main());
      }
    }
  }

  ImGui::EndChild();
}

bool AssetViewerWindow::ProcessEvent(const SDL_Event& event) {
  if (event.type == SDL_DROPFILE) {
    ImportAsset(event.drop.file);
    SDL_free(event.drop.file);
    return true;
  }

  return false;
}

AssetEditor* AssetViewerWindow::OpenAssetEditor(const std::string& asset_id) {
  auto editor = scene()->GetController<AssetEditor>(AssetEditor::GetAssetEditorId(asset_id));
  if (editor) {
    editor->Focus();
    return editor;
  }

  const std::string asset_type = GetApplicationAssetLibrary()->GetAssetType(asset_id);
  if (asset_type == "scene") {
    return scene()->AddController<SceneEditor>(asset_id);
  } else if (asset_type == "scene_controller") {
    return scene()->AddController<ScriptEditor>(asset_id);
  } else if (asset_type == "texture2d") {
    return scene()->AddController<TextureEditor>(asset_id);
  } else if (asset_type == "settings") {
    return scene()->AddController<SettingsEditor>(asset_id);
  } else {
    LogE("Unknown asset type: {}", asset_type);
    return nullptr;
  }
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
