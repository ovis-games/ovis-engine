#pragma once

#include "../ui_window.hpp"

namespace ove
{
  
class ModalWindow : public UiWindow {
public:
  ModalWindow(const std::string& id, const std::string& window_title = "", ImGuiWindowFlags window_flags = 0);

  void DrawImGui() override;
};

} // namespace ove
