#pragma once

#include "asset_editors/asset_editor.hpp"

#include <ovis/core/json.hpp>

namespace ove {

class AssetViewerWindow {
 public:
  AssetViewerWindow(AssetEditors* open_editors);
  void Draw();

 private:
  AssetEditors* open_editors_;
  std::string current_path_;

  AssetEditor* OpenAssetEditor(const std::string& asset_id);
  std::string GetNewAssetName(const std::string& base_name) const;
};

}  // namespace ove