#pragma once

#include "asset_editor.hpp"

#include <TextEditor.h>

namespace ovis {
namespace editor {

struct LuaError {
  std::string asset_id;
  int line;
  std::string message;
};
std::vector<LuaError> ParseLuaErrorMessage(std::string_view error_message);

class ScriptEditor : public AssetEditor {
 public:
  ScriptEditor(const std::string& script_id);

  void DrawContent() override;
  void Save() override;
  bool CanUndo() const override { return editor_.CanUndo(); }
  void Undo() override { editor_.Undo(); }
  bool CanRedo() const override { return editor_.CanRedo(); }
  void Redo() override { editor_.Redo(); }
  inline bool HasUnsavedChanges() const override { return text_changed_; }

  void ClearErrors() { SetErrors({}); }
  void SetErrors(const std::vector<LuaError>& errors);

  static void CreateNew(const std::string& asset_id);

 private:
  TextEditor editor_;
  bool text_changed_ = false;
  // Indicates whether the text was just loaded and passed to the TextEditor. This is used to avoid a false positive for
  // the text_changed_ variable.
  bool text_just_loaded_ = false;
};

}  // namespace editor
}  // namespace ovis
