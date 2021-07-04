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

    bool DrawAction(json& action, size_t index);
    void DrawSpace(size_t index);
};
}  // namespace editor
}  // namespace ovis
