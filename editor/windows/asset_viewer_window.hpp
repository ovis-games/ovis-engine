#pragma once

#include "asset_editors/asset_editor.hpp"
#include <ovis/imgui/imgui_window.hpp>

#include <ovis/utils/json.hpp>

namespace ovis {
namespace editor {

class AssetViewerWindow : public ImGuiWindow {
 public:
  AssetViewerWindow();

  void DrawContent() override;
  // bool ProcessEvent(const SDL_Event& event) override;

 private:
  std::string current_path_;

  AssetEditor* OpenAssetEditor(const std::string& asset_id);
  bool CloseAssetEditor(const std::string& asset_id);
  std::string GetNewAssetName(const std::string& base_name) const;
};

}  // namespace editor
}  // namespace ovis