#include "visual_script_editor.hpp"

#include <fstream>
#include <regex>

#include <ovis/utils/file.hpp>
#include <ovis/utils/range.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/script_scene_controller.hpp>
#include <ovis/input/key.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>
#include <ovis/application/application.hpp>

namespace ovis {
namespace editor {

VisualScriptEditor::VisualScriptEditor(const std::string& script_id) : ScriptLibraryEditor(script_id) {
}

void VisualScriptEditor::DrawContent() {
  ScriptLibraryEditor::DrawContent();
}

void VisualScriptEditor::CreateNew(const std::string& asset_id) {
  // GetApplicationAssetLibrary()->CreateAsset(asset_id, "scene_controller", {std::make_pair("lua", lua_code)});
}

}  // namespace editor
}  // namespace ovis