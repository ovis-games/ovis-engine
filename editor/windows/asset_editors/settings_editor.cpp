#include "settings_editor.hpp"

#include "../../imgui_extensions/input_json.hpp"
#include "imgui_stdlib.h"

#include <ovis/application/game_settings.hpp>

namespace ovis {
namespace editor {

SettingsEditor::SettingsEditor(const std::string& settings_id) : AssetEditor(settings_id) {
  SetupJsonFile(GameSettings{});
}

void SettingsEditor::DrawContent() {
  json settings = settings_;
  if (ImGui::InputJson("Game Settings", &settings, GameSettings::SCHEMA, nullptr, ImGuiInputJsonFlags_IgnoreEnclosingObject)) {
    settings_ = settings;
    SubmitJsonFile(settings_);
  }
}

void SettingsEditor::DrawInspectorContent() {}

void SettingsEditor::Save() {
  SaveFile("json", settings_.dump());
}

void SettingsEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  if (file_type == "json") {
    settings_ = data;
  }
}

}  // namespace editor
}  // namespace ovis