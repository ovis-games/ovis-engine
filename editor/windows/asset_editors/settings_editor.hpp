#pragma once

#include "asset_editor.hpp"

namespace ove {

class SettingsEditor : public AssetEditor {
 public:
  SettingsEditor(const std::string& id);

  void DrawContent() override;
  void DrawInspectorContent() override;

  void Save() override;

 private:
  ovis::json settings_;

  void JsonFileChanged(const ovis::json& data, const std::string& file_type) override;
};

}  // namespace ove
