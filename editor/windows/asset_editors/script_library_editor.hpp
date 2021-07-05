#pragma once

#include <ovis/core/scripting.hpp>
#include "asset_editor.hpp"

namespace ovis {
namespace editor {

class ScriptLibraryEditor : public AssetEditor {
  public:
    ScriptLibraryEditor(const std::string& asset_id);

    void DrawContent() override;
    void Save() override {}

  private:
    ScriptChunk chunk_;
    json editing_copy_;
    
    bool DrawActions(json::json_pointer path);
    bool DrawAction(json::json_pointer path);
    bool DrawFunctionCall(json::json_pointer path);
    bool DrawIfStatement(json::json_pointer path);
    void DrawSpace(json::json_pointer path);
};
}  // namespace editor
}  // namespace ovis
