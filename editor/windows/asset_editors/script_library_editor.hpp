#pragma once

#include "asset_editor.hpp"

#include <ovis/core/scripting.hpp>

namespace ovis {
namespace editor {

class ScriptLibraryEditor : public AssetEditor {
 public:
  ScriptLibraryEditor(const std::string& asset_id);

  void DrawContent() override;
  void Save() override { SaveFile("json", GetCurrentJsonFileState().dump()); }

 private:
  ScriptChunk chunk_;
  json editing_copy_;

  std::map<std::string, json> docs_;
  json::json_pointer current_edit_path_;
  bool start_editing_ = false;
  std::string highlighted_reference_;
  std::string reference_to_highlight_;

  bool DrawActions(const json::json_pointer& path);
  bool DrawAction(const json::json_pointer& path);
  bool DrawFunctionCall(const json::json_pointer& path);
  bool DrawIfStatement(const json::json_pointer& path);
  bool DrawNewAction(const json::json_pointer& path);
  void DrawSpace(const json::json_pointer& path);
  bool DrawInput(const json::json_pointer& path, ScriptType type);

  void RemoveAction(const json::json_pointer& path);
  void JsonFileChanged(const json& data, const std::string& file_type) override;
  json::json_pointer GetActionWithId(size_t id, json::json_pointer base_path = json::json_pointer{"/actions"});
  std::string GetReferenceName(std::string_view reference);
};
}  // namespace editor
}  // namespace ovis
