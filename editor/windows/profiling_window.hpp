#pragma once

#include <ovis/imgui/imgui_window.hpp>

namespace ovis {
namespace editor {

class ProfilingWindow : public ImGuiWindow {
 public:
  ProfilingWindow();

  void DrawContent() override;

 private:
};

}  // namespace editor
}  // namespace ovis
