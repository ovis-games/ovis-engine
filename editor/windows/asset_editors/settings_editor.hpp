#pragma once

#include "asset_editor.hpp"

namespace ove {

class SettingsEditor : public AssetEditor {
 public:
  SettingsEditor(const std::string& id);

  void DrawContent() override;
  void DrawInspectorContent() override;

  void Save() override;
  ActionHistoryBase* GetActionHistory() override { return &action_history_; }

 private:
  ActionHistory<SettingsEditor> action_history_;
  std::string startup_scene_ = " asd";
};

}  // namespace ove
