#pragma once

#include "windows/asset_editors/asset_editor.hpp"
#include "windows/asset_viewer_window.hpp"
#include "windows/log_window.hpp"

#include <ovis/core/asset_library.hpp>

#include <ovis/engine/scene_controller.hpp>

namespace ove {

class EditorWindowController : public ovis::SceneController {
 public:
  EditorWindowController(const std::vector<std::string>* log_history);

  void Update(std::chrono::microseconds delta_time) override;
  void DrawImGui() override;
  bool ProcessEvent(const SDL_Event& event) override;

 private:
  AssetEditors open_editors_;
  LogWindow log_window_;
  AssetViewerWindow asset_viewer_window_;

  void Save();
  void Undo();
  void Redo();
};

}  // namespace ove