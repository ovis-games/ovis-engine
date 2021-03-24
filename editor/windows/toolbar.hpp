#pragma once

#include <ovis/imgui/imgui_window.hpp>

#include <imgui.h>

#include <ovis/graphics/texture2d.hpp>

namespace ovis {
namespace editor {

class Toolbar : public ImGuiWindow {
 public:
  Toolbar();

  void ProcessEvent(Event* event) override;

 protected:
  void BeforeBegin() override;
  void DrawContent() override;

 private:
  struct Icons {
    std::unique_ptr<Texture2D> save;
    std::unique_ptr<Texture2D> undo;
    std::unique_ptr<Texture2D> redo;
    std::unique_ptr<Texture2D> package;
    std::unique_ptr<Texture2D> windows;
  } icons_;
  ImVec2 icon_size_ = {28, 28};

  void Save();
  void Undo();
  void Redo();
};

}  // namespace editor
}  // namespace ovis
