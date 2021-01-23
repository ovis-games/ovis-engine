#pragma once

#include "ui_window.hpp"

#include <imgui.h>

#include <ovis/graphics/texture2d.hpp>

namespace ove {

class Toolbar : public UiWindow {
 public:
  Toolbar();

 protected:
  void BeforeBegin() override;
  void DrawContent() override;

 private:
  struct Icons {
    std::unique_ptr<ovis::Texture2D> save;
    std::unique_ptr<ovis::Texture2D> undo;
    std::unique_ptr<ovis::Texture2D> redo;
    std::unique_ptr<ovis::Texture2D> package;
    std::unique_ptr<ovis::Texture2D> windows;
  } icons_;
  ImVec2 icon_size_ = {28, 28};

  void Save();
  void Undo();
  void Redo();
};

}  // namespace ove
