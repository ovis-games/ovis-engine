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
    std::shared_ptr<Texture2D> save;
    std::shared_ptr<Texture2D> undo;
    std::shared_ptr<Texture2D> redo;
    std::shared_ptr<Texture2D> package;
    std::shared_ptr<Texture2D> windows;
  } icons_;
  ImVec2 icon_size_ = {28, 28};

  void Save();
  void Undo();
  void Redo();
};

}  // namespace editor
}  // namespace ovis
