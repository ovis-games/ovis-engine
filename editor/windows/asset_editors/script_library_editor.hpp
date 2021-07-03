#pragma once

#include "asset_editor.hpp"

namespace ovis {
namespace editor {

class ScriptLibraryEditor : public AssetEditor {
  public:
    ScriptLibraryEditor(const std::string& asset_id);

    void DrawContent() override;
    void Save() override {}

  private:
    std::vector<json> actions_;

    void DrawAction(json& action, size_t index);
    void DrawSpace(size_t index);
};
}  // namespace editor
}  // namespace ovis
