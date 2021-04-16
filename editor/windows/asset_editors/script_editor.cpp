#include "script_editor.hpp"

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

std::vector<LuaError> ParseLuaErrorMessage(std::string_view error_message) {
  std::vector<LuaError> errors;
  std::regex lua_error_regex("^(.+):(\\d+): (.+)");

  for (const auto& match : make_range(std::cregex_iterator(error_message.begin(), error_message.end(), lua_error_regex),
                                      std::cregex_iterator())) {
    errors.push_back({match[1].str(), std::stoi(match[2].str()), match[3].str()});
  }

  return errors;
}

ScriptEditor::ScriptEditor(const std::string& script_id) : AssetEditor(script_id) {
  auto lang = TextEditor::LanguageDefinition::Lua();

  editor_.SetLanguageDefinition(lang);
  editor_.SetShowWhitespaces(false);
  editor_.SetTabSize(3);

  std::optional<std::string> lua_code = LoadTextFile("lua");
  if (lua_code) {
    editor_.SetText(*lua_code);
  }
}

void ScriptEditor::DrawContent() {
  auto start_frame_controller = scene()->GetController<ImGuiStartFrameController>();
  SDL_assert(start_frame_controller != nullptr);

  ImGui::PushFont(start_frame_controller->GetFont("Inconsolata-Regular"));
  editor_.Render("ScriptEditor");
  ImGui::PopFont();

  ImGuiIO& io = ImGui::GetIO();
  auto shift = io.KeyShift;
  auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
  auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;
  if (!editor_.IsReadOnly() && ctrl && !shift && !alt && ImGui::IsKeyPressed(Key::S().code)) {
    Save();
  }
}

void ScriptEditor::Save() {
  const std::string code = editor_.GetText();
  SaveFile("lua", code);

  sol::protected_function_result result = lua.do_string(code, "=" + asset_id());

  std::vector<LuaError> errors;
  if (!result.valid()) {
    const std::string error_message = result;
    LogE("Failed to load script: {}", error_message);
    if (error_message.length() > 0) {
      errors = ParseLuaErrorMessage(error_message);
      SetErrors(errors);
    }
  } else {
    SetErrors({});
  }
}

void ScriptEditor::SetErrors(const std::vector<LuaError>& errors) {
  std::map<int, std::string> error_markers;

  for (const auto& error : errors) {
    SDL_assert(error.asset_id == asset_id());
    if (error.asset_id == asset_id()) {
      error_markers.insert(std::make_pair(error.line, error.message));
    }
  }

  editor_.SetErrorMarkers(error_markers);
}

void ScriptEditor::CreateNew(const std::string& asset_id) {
  std::string scene_controller_name;
  std::copy_if(
      asset_id.begin(), asset_id.end(), std::back_insert_iterator<std::string>(scene_controller_name),
      [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });

  if (scene_controller_name.size() == 0 || (scene_controller_name[0] >= '0' && scene_controller_name[0] <= '9')) {
    scene_controller_name = '_' + scene_controller_name;
  }

  const std::string lua_code = fmt::format(R"LUA({0} = class('{0}')

function {0}:Play()
  LogI('{0}:Play()')
end

function {0}:Update(delta_time)
  LogI('{0}:Update()')
end

return {0})LUA",
                                           scene_controller_name);

  GetApplicationAssetLibrary()->CreateAsset(asset_id, "scene_controller", {std::make_pair("lua", lua_code)});
}

}  // namespace editor
}  // namespace ovis