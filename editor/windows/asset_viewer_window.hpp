#pragma once

#include "asset_editors/asset_editor.hpp"
#include "ui_window.hpp"

#include <ovis/core/json.hpp>

namespace ove {

class AssetViewerWindow : public UiWindow {
 public:
  AssetViewerWindow();

  void DrawContent() override;
  bool ProcessEvent(const SDL_Event& event) override;

 private:
  std::string current_path_;

  AssetEditor* OpenAssetEditor(const std::string& asset_id);
  std::string GetNewAssetName(const std::string& base_name) const;
};

}  // namespace ove