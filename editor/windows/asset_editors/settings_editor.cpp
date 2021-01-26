#include "settings_editor.hpp"
#include "imgui_stdlib.h"
#include "../../imgui_extensions/input_asset.hpp"

namespace ove {

SettingsEditor::SettingsEditor(const std::string& settings_id) : AssetEditor(settings_id) {
}

void SettingsEditor::DrawContent() {
  if (ImGui::InputAsset("Startup Scene", &startup_scene_, "scene")) {
    ovis::LogD("New startup scene: {}", startup_scene_);
  }
}

void SettingsEditor::DrawInspectorContent() {
}

void SettingsEditor::Save() {}

}  // namespace ove