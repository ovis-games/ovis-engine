#pragma once

#include "../../action_history.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <SDL2/SDL.h>
#include <imgui.h>

#include <ovis/utils/file.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/imgui/imgui_window.hpp>

namespace ovis {
namespace editor {

class AssetEditor : public ImGuiWindow {
 public:
  AssetEditor(const std::string& asset_id);
  virtual ~AssetEditor();

  inline std::string asset_id() const { return asset_id_; }

  void SaveFile(const std::string& type, const std::variant<std::string, Blob>& content);
  std::optional<std::string> LoadTextFile(const std::string& file_type);
  std::optional<Blob> LoadBinaryFile(const std::string& file_type);

  virtual void DrawInspectorContent() {}
  virtual void Save() = 0;

  virtual bool CanUndo() const { return current_undo_redo_position_ != changes_.begin(); }
  virtual void Undo();
  virtual bool CanRedo() const {
    return current_undo_redo_position_ != changes_.end();
    ;
  }
  virtual void Redo();
  virtual bool HasUnsavedChanges() const;
  void Close() override;

  void ProcessEvent(Event* event) override;
  void Update(std::chrono::microseconds) override;
  static AssetEditor* last_focused_document_window;

  static const std::string GetAssetEditorId(const std::string& asset_id);

 protected:
  void BeforeBegin() final;

  void SetupJsonFile(const json& default_data, const std::string& file_type = "json");
  void SubmitJsonFile(const json& data, const std::string& file_type = "json");
  json GetCurrentJsonFileState(const std::string& file_type = "json");
  virtual void JsonFileChanged(const json& data, const std::string& file_type) {}

 private:
  std::string asset_id_;

  struct JsonFileChange {
    std::string file_type;
    json undo_patch;
    json redo_patch;
  };
  using FileChange = std::variant<JsonFileChange>;
  std::vector<FileChange> changes_;
  std::vector<FileChange>::iterator current_undo_redo_position_;

  struct JsonFile {
    json data;
    bool unsaved_changes;
  };
  std::map<std::string, JsonFile> json_files_;
};

}  // namespace editor
}  // namespace ovis