#include "asset_editor.hpp"

#include <ovis/core/asset_library.hpp>

namespace ove {

AssetEditor* AssetEditor::last_focused_document_window = nullptr;

AssetEditor::AssetEditor(const std::string& asset_id)
    : UiWindow(GetAssetEditorId(asset_id), asset_id),
      asset_id_(asset_id),
      current_undo_redo_position_(changes_.begin()) {
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

void AssetEditor::Undo() {
  if (current_undo_redo_position_ != changes_.begin()) {
    --current_undo_redo_position_;
    if (std::holds_alternative<JsonFileChange>(*current_undo_redo_position_)) {
      const JsonFileChange& json_change = std::get<JsonFileChange>(*current_undo_redo_position_);
      ovis::json& data = json_files_[json_change.file_type];
      JsonFileChanged(data = data.patch(json_change.undo_patch), json_change.file_type);
    }
  }
}

void AssetEditor::Redo() {
  if (current_undo_redo_position_ != changes_.end()) {
    if (std::holds_alternative<JsonFileChange>(*current_undo_redo_position_)) {
      const JsonFileChange& json_change = std::get<JsonFileChange>(*current_undo_redo_position_);
      ovis::json& data = json_files_[json_change.file_type];
      JsonFileChanged(data = data.patch(json_change.redo_patch), json_change.file_type);
    }
    ++current_undo_redo_position_;
  }
}

void AssetEditor::DrawImGui() {
  UiWindow::DrawImGui();
  if (has_focus()) {
    last_focused_document_window = this;
  }
}

const std::string AssetEditor::GetAssetEditorId(const std::string& asset_id) {
  return "AssetEditor - " + asset_id;
}

void AssetEditor::SetupJsonFile(const ovis::json& default_data, const std::string& file_type) {
  std::optional<std::string> json_text = ovis::GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id(), file_type);
  JsonFileChanged(json_files_[file_type] = json_text ? ovis::json::parse(*json_text) : default_data, file_type);
}

void AssetEditor::SubmitJsonFile(const ovis::json& data, const std::string& file_type) {
  ovis::json& current_data = json_files_[file_type];
  const ovis::json redo_patch = ovis::json::diff(current_data, data);
  const ovis::json undo_patch = ovis::json::diff(data, current_data);
  changes_.erase(current_undo_redo_position_, changes_.end());
  changes_.push_back(JsonFileChange{file_type, undo_patch, redo_patch});
  current_undo_redo_position_ = changes_.end();
  ovis::LogD("Scene patch: {}", redo_patch.dump());
  ovis::LogD("Scene undo patch: {}", undo_patch.dump());
  current_data = data;
}

}  // namespace ove