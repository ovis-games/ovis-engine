#pragma once

#include "../ui_window.hpp"

namespace ove
{
  
class ModalWindow : public UiWindow {
public:
  ModalWindow(const std::string& id, const std::string& window_title = "");

  void DrawImGui() override;
};

} // namespace ove
