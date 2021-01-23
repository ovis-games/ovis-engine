#pragma once

#include <imgui.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/engine/scene_controller.hpp>

namespace ove {

class EditorWindowController : public ovis::SceneController {
 public:
  EditorWindowController();

  void Update(std::chrono::microseconds delta_time) override;
  void DrawImGui() override;
  bool ProcessEvent(const SDL_Event& event) override;

 private:
  struct Icons {
    std::unique_ptr<ovis::Texture2D> save;
    std::unique_ptr<ovis::Texture2D> undo;
    std::unique_ptr<ovis::Texture2D> redo;
    std::unique_ptr<ovis::Texture2D> package;
    std::unique_ptr<ovis::Texture2D> windows;
  } icons_;
  ImVec2 icon_size_ = {28, 28};

  void DrawDockSpace();
  void DrawToolbar();

  void Save();
  void Undo();
  void Redo();
};

}  // namespace ove