#pragma once

#include <ovis/core/scripting.hpp>
#include "asset_editor.hpp"

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
    
    bool DrawActions(json::json_pointer path);
    bool DrawAction(json::json_pointer path);
    bool DrawFunctionCall(json::json_pointer path);
    bool DrawIfStatement(json::json_pointer path);
    void DrawSpace(json::json_pointer path);
};
}  // namespace editor
}  // namespace ovis
