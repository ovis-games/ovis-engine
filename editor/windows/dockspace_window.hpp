#pragma once

#include "ui_window.hpp"

#include <imgui.h>

namespace ovis {
namespace editor {

class DockspaceWindow : public UiWindow {
 public:
  DockspaceWindow();

  inline ImGuiID dockspace_main() const { return dockspace_main_; }
  inline ImGuiID dockspace_left() const { return dockspace_left_; }
  inline ImGuiID dockspace_right() const { return dockspace_right_; }
  inline ImGuiID dockspace_bottom() const { return dockspace_bottom_; }

 protected:
  void BeforeBegin() override;
  void DrawContent() override;

 private:
  ImGuiID dockspace_main_;
  ImGuiID dockspace_left_;
  ImGuiID dockspace_right_;
  ImGuiID dockspace_bottom_;
};

}  // namespace editor
}  // namespace ovis
