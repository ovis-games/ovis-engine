#pragma once

#include <ovis/imgui/imgui_window.hpp>

namespace ovis {
namespace editor {

class ModalWindow : public ImGuiWindow {
 public:
  ModalWindow(const std::string& id, const std::string& window_title = "");

  void Update(std::chrono::microseconds delta_time) override;
};

}  // namespace editor
}  // namespace ovis