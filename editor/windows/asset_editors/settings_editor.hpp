#pragma once

#include "asset_editor.hpp"

namespace ovis {
namespace editor {

class SettingsEditor : public AssetEditor {
 public:
  SettingsEditor(const std::string& id);

  void DrawContent() override;
  void DrawInspectorContent() override;

  void Save() override;

 private:
  json settings_;

  void JsonFileChanged(const json& data, const std::string& file_type) override;
};

}  // namespace editor
}  // namespace ovis
