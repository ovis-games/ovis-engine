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
  std::string startup_scene_ = " asd";
};

}  // namespace ove
