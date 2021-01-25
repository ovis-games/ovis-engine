#include "asset_editor.hpp"

#include <ovis/core/asset_library.hpp>

namespace ove {

AssetEditor* AssetEditor::last_focused_document_window = nullptr;

AssetEditor::AssetEditor(const std::string& asset_id) : UiWindow(GetAssetEditorId(asset_id), asset_id), asset_id_(asset_id) {
  UpdateAfter("Dockspace Window");
}

AssetEditor::~AssetEditor() {
  if (last_focused_document_window == this) {
    last_focused_document_window = nullptr;
  }
}

void AssetEditor::SaveFile(const std::string& type, const std::variant<std::string, ovis::Blob>& content) {
  if (!ovis::GetApplicationAssetLibrary()->SaveAssetFile(asset_id(), type, content)) {
    ovis::LogE("Failed to save file '{}' for asset '{}'", type, asset_id());
  }
}

std::optional<std::string> AssetEditor::LoadTextFile(const std::string& file_type) {
  return ovis::GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id(), file_type);
}

std::optional<ovis::Blob> AssetEditor::LoadBinaryFile(const std::string& file_type) {
  return ovis::GetApplicationAssetLibrary()->LoadAssetBinaryFile(asset_id(), file_type);
}

void AssetEditor::DrawImGui() {
  UiWindow::DrawImGui();
  if (has_focus()) {
    last_focused_document_window = this;
  }
}

// void AssetEditor::Draw(ImGuiID dockspace_id) {
//   if (first_frame_) {
//     ImGui::SetNextWindowDockID(dockspace_id);
//     first_frame_ = false;
//   }

//   bool is_active = ImGui::Begin(asset_id().c_str(), &keep_open_);
//   if (should_focus_) {
//     ImGui::SetWindowFocus();
//     should_focus_ = false;
//   }
//   CheckForFocus();

//   if (is_active && keep_open_) {
//     Draw();
//   }
//   ImGui::End();
// }

// void AssetEditor::CheckForFocus() {
//   const bool was_focused = is_focused_;
//   is_focused_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
//   if (!was_focused && is_focused_) {
//     last_focused_document_window = this;
//   }
// }

const std::string AssetEditor::GetAssetEditorId(const std::string& asset_id) {
  return "AssetEditor - " + asset_id;
}

}  // namespace ove