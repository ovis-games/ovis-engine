#include "script_editor.hpp"

#include <fstream>
#include <regex>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/file.hpp>
#include <ovis/core/range.hpp>

#include <ovis/engine/engine.hpp>
#include <ovis/engine/lua.hpp>

namespace ove {

std::vector<LuaError> ParseLuaErrorMessage(const std::string& error_message) {
  std::vector<LuaError> errors;
  std::regex lua_error_regex("^(.+):(\\d+): (.+)");

  for (const auto& match : ovis::make_range(
           std::sregex_iterator(error_message.begin(), error_message.end(), lua_error_regex), std::sregex_iterator())) {
    errors.push_back({match[1].str(), std::stoi(match[2].str()), match[3].str()});
  }

  return errors;
}

ScriptEditor::ScriptEditor(const std::string& script_id) : AssetEditor(script_id), action_history_(this) {
  auto lang = TextEditor::LanguageDefinition::Lua();

  editor_.SetLanguageDefinition(lang);
  editor_.SetShowWhitespaces(false);

  std::optional<std::string> lua_code = LoadTextFile("lua");
  if (lua_code) {
    editor_.SetText(*lua_code);
  }
}

void ScriptEditor::Draw() {
  editor_.Render("ScriptEditor");
}

void ScriptEditor::Save() {
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

void ScriptEditor::SetErrors(const std::vector<LuaError>& errors) {
  std::map<int, std::string> error_markers;

  for (const auto& error : errors) {
    SDL_assert(error.asset_id == asset_id());
    error_markers.insert(std::make_pair(error.line, error.message));
  }

  editor_.SetErrorMarkers(error_markers);
}

}  // namespace ove