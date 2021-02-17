#pragma once

#include "ui_window.hpp"

namespace ovis {
namespace editor {

class InspectorWindow : public UiWindow {
 public:
  InspectorWindow();

 protected:
  void DrawContent() override;

 private:
};

}  // namespace editor
}  // namespace ovis