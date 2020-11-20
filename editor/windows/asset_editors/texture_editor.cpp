#include <fstream>
#include <regex>

#include "../editor_window.hpp"
#include "script_editor.hpp"

#include <ovis/core/asset_library.hpp>
#include <ovis/core/file.hpp>
#include <ovis/core/range.hpp>

#include <ovis/engine/engine.hpp>
#include <ovis/engine/lua.hpp>

namespace ove {

TextureEditor::TextureEditor(const std::string& script_id) : AssetEditor(script_id), action_history_(this) {
  texture_ = EditorWindow::instance()->resource_manager()->Load<ovis::Texture2D>()
}

void TextureEditor::Draw() {
  editor_.Render("TextureEditor");
}

void TextureEditor::Save() {
  SaveFile("lua", editor_.GetText());
  sol::protected_function_result result = ovis::Lua::AddSceneController(editor_.GetText(), asset_id());

  std::vector<LuaError> errors;
  if (!result.valid()) {
    const std::string error_message = result;
    ovis::LogE("Failed to load script: {}", error_message);
    errors = ParseLuaErrorMessage(error_message);
  }

  SetErrors(errors);
}

void TextureEditor::SetErrors(const std::vector<LuaError>& errors) {
  std::map<int, std::string> error_markers;

  for (const auto& error : errors) {
    SDL_assert(error.asset_id == asset_id());
    error_markers.insert(std::make_pair(error.line, error.message));
  }

  editor_.SetErrorMarkers(error_markers);
}

}  // namespace ove