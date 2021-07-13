#pragma once

#include "asset_editor.hpp"
#include "script_library_editor.hpp"

namespace ovis {
namespace editor {

class VisualScriptEditor : public ScriptLibraryEditor {
 public:
  VisualScriptEditor(const std::string& script_id);

  void DrawContent() override;

  static void CreateNew(const std::string& asset_id);

 private:
};

}  // namespace editor
}  // namespace ovis
