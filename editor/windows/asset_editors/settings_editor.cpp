#include "settings_editor.hpp"

#include "../../imgui_extensions/input_json.hpp"
#include "imgui_stdlib.h"

#include <ovis/engine/game_settings.hpp>

namespace ove {

SettingsEditor::SettingsEditor(const std::string& settings_id) : AssetEditor(settings_id) {
  SetupJsonFile(ovis::GameSettings{});
}

void SettingsEditor::DrawContent() {
  ovis::json settings = settings_;
  if (ImGui::InputJson("Game Settings", &settings, ovis::GameSettings::SCHEMA, ImGuiInputJsonFlags_IgnoreEnclosingObject)) {
    settings_ = settings;
    SubmitJsonFile(settings_);
  }
}

void SettingsEditor::DrawInspectorContent() {}

void SettingsEditor::Save() {
  SaveFile("json", settings_.dump());
}

void SettingsEditor::JsonFileChanged(const ovis::json& data, const std::string& file_type) {
  if (file_type == "json") {
    settings_ = data;
  }
}

}  // namespace ove