#pragma once

#include <TextEditor.h>

#include "asset_editor.hpp"

namespace ove {

struct LuaError {
  std::string asset_id;
  int line;
  std::string message;
};
std::vector<LuaError> ParseLuaErrorMessage(const std::string& error_message);

class ScriptEditor : public AssetEditor {
 public:
  ScriptEditor(const std::string& script_id);

  void DrawContent() override;
  void Save() override;
  bool CanUndo() const override { return editor_.CanUndo(); }
  void Undo() override { editor_.Undo(); }
  bool CanRedo() const override { return editor_.CanRedo(); }
  void Redo() override { editor_.Redo(); }

  void ClearErrors() { SetErrors({}); }
  void SetErrors(const std::vector<LuaError>& errors);

 private:
  TextEditor editor_;
};

}  // namespace ove
