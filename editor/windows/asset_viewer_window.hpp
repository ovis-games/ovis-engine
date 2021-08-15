#pragma once

#include "asset_editors/asset_editor.hpp"

#include <ovis/utils/json.hpp>
#include <ovis/imgui/imgui_window.hpp>

namespace ovis {
namespace editor {

class AssetViewerWindow : public ImGuiWindow {
 public:
  AssetViewerWindow();

  void DrawContent() override;
  void ProcessEvent(Event* event) override;

  AssetEditor* OpenAssetEditor(const std::string& asset_id);

 private:
  std::string current_path_;

  bool CloseAssetEditor(const std::string& asset_id);
  std::string GetNewAssetName(const std::string& base_name) const;
};

}  // namespace editor
}  // namespace ovis
