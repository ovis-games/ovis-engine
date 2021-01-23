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
  ActionHistoryBase* GetActionHistory() override { return &action_history_; }

  void ClearErrors() { SetErrors({}); }
  void SetErrors(const std::vector<LuaError>& errors);

 private:
  TextEditor editor_;
  ActionHistory<ScriptEditor> action_history_;
};

}  // namespace ove
