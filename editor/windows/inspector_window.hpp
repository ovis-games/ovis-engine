#pragma once

#include <ovis/imgui/imgui_window.hpp>

namespace ovis {
namespace editor {

class InspectorWindow : public ImGuiWindow {
 public:
  InspectorWindow();

 protected:
  void DrawContent() override;

 private:
};

}  // namespace editor
}  // namespace ovis