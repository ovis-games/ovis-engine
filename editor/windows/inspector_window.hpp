#pragma once

#include "ui_window.hpp"

namespace ove {

class InspectorWindow : public UiWindow {
 public:
  InspectorWindow();

 protected:
  void DrawContent() override;

 private:
};

}  // namespace ove