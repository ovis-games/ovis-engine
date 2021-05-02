#include "asset_editor.hpp"

#include "../../editor_window.hpp"

#include <ovis/core/asset_library.hpp>
#include <ovis/input/key_events.hpp>

namespace ovis {
namespace editor {

AssetEditor* AssetEditor::last_focused_document_window = nullptr;

AssetEditor::AssetEditor(const std::string& asset_id)
    : ImGuiWindow(GetAssetEditorId(asset_id), asset_id),
      asset_id_(asset_id),
      current_undo_redo_position_(changes_.begin()) {
  SubscribeToEvent(KeyPressEvent::TYPE);
  UpdateAfter("Dockspace Window");
}

AssetEditor::~AssetEditor() {
  if (last_focused_document_window == this) {
    last_focused_document_window = nullptr;
  }
}

void AssetEditor::SaveFile(const std::string& type, const std::variant<std::string, Blob>& content) {
  if (!GetApplicationAssetLibrary()->SaveAssetFile(asset_id(), type, content)) {
    LogE("Failed to save file '{}' for asset '{}'", type, asset_id());
  } else if (json_files_.contains(type)) {
    json_files_[type].unsaved_changes = false;
  }
}

std::optional<std::string> AssetEditor::LoadTextFile(const std::string& file_type) {
  return GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id(), file_type);
}

std::optional<Blob> AssetEditor::LoadBinaryFile(const std::string& file_type) {
  return GetApplicationAssetLibrary()->LoadAssetBinaryFile(asset_id(), file_type);
}

void AssetEditor::Undo() {
  if (current_undo_redo_position_ != changes_.begin()) {
    --current_undo_redo_position_;
    if (std::holds_alternative<JsonFileChange>(*current_undo_redo_position_)) {
      const JsonFileChange& json_change = std::get<JsonFileChange>(*current_undo_redo_position_);
      json& data = json_files_[json_change.file_type].data;
      json_files_[json_change.file_type].unsaved_changes = true;
      JsonFileChanged(data = data.patch(json_change.undo_patch), json_change.file_type);
    }
  }
}

void AssetEditor::Redo() {
  if (current_undo_redo_position_ != changes_.end()) {
    if (std::holds_alternative<JsonFileChange>(*current_undo_redo_position_)) {
      const JsonFileChange& json_change = std::get<JsonFileChange>(*current_undo_redo_position_);
      json& data = json_files_[json_change.file_type].data;
      json_files_[json_change.file_type].unsaved_changes = true;
      JsonFileChanged(data = data.patch(json_change.redo_patch), json_change.file_type);
    }
    ++current_undo_redo_position_;
  }
}

bool AssetEditor::HasUnsavedChanges() const {
  for (const auto& json_file : json_files_) {
    if (json_file.second.unsaved_changes) {
      return true;
    }
  }
  return false;
}

void AssetEditor::Close() {
  if (HasUnsavedChanges()) {
    EditorWindow::instance()->ShowMessageBox(
        fmt::format("Unsaved changes in {}", asset_id()),
        fmt::format("There are unsaved changes in {}. If you close the editor, these changes will be lost.",
                    asset_id()),
        {"Discard changes and close", "Save and close", "Cancel"},
        [asset_editor = safe_ptr(this)](std::string_view button) {
          if (asset_editor == nullptr) {
            return;
          }
          if (button == "Discard changes and close") {
            asset_editor->Remove();
          } else if (button == "Save and close") {
            asset_editor->Save();
            asset_editor->Remove();
          }
        });
  } else {
    Remove();
  }
}

void AssetEditor::ProcessEvent(Event* event) {
  if (event->type() == KeyPressEvent::TYPE) {
    auto key_press_event = static_cast<KeyPressEvent*>(event);
    if (key_press_event->key() == Key::S() && (IsKeyPressed(Key::ControlLeft()) || IsKeyPressed(Key::ControlRight()))) {
      Save();
      event->StopPropagation();
    }
  }
}

void AssetEditor::Update(std::chrono::microseconds delta_time) {
  ImGuiWindow::Update(delta_time);
  if (has_focus()) {
    last_focused_document_window = this;
  }
}

const std::string AssetEditor::GetAssetEditorId(const std::string& asset_id) {
  return "AssetEditor - " + asset_id;
}

void AssetEditor::BeforeBegin() {
  if (HasUnsavedChanges()) {
    AddFlags(ImGuiWindowFlags_UnsavedDocument);
  } else {
    RemoveFlags(ImGuiWindowFlags_UnsavedDocument);
  }
}

void AssetEditor::SetupJsonFile(const json& default_data, const std::string& file_type) {
  std::optional<std::string> json_text = GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id(), file_type);
  JsonFileChanged(json_files_[file_type].data = json_text ? json::parse(*json_text) : default_data, file_type);
}

void AssetEditor::SubmitJsonFile(const json& data, const std::string& file_type) {
  json& current_data = json_files_[file_type].data;
  const json redo_patch = json::diff(current_data, data);
  const json undo_patch = json::diff(data, current_data);

  if (redo_patch.size() == 0 && undo_patch.size() == 0) {
    LogD("No changes detected!");
  } else {
    json_files_[file_type].unsaved_changes = true;
    changes_.erase(current_undo_redo_position_, changes_.end());
    changes_.push_back(JsonFileChange{file_type, undo_patch, redo_patch});
    current_undo_redo_position_ = changes_.end();
    LogD("Scene patch: {}", redo_patch.dump());
    LogD("Scene undo patch: {}", undo_patch.dump());
    current_data = data;
  }
}

json AssetEditor::GetCurrentJsonFileState(const std::string& file_type) {
  return json_files_[file_type].data;
}

}  // namespace editor
}  // namespace ovis