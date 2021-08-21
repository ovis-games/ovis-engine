#pragma once

#include <ovis/application/window.hpp>
#include <ovis/editor_viewport/camera_controller.hpp>

namespace ovis {
namespace editor {

class EditorViewport : public Window {
 public:
  EditorViewport();

  void SetSelection(std::string_view object_path);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  std::optional<std::string> selected_object_;
  std::vector<ViewportController*> controllers_;
  CameraController camera_controller_;

  void AddController(ViewportController* controller);
};

}
}
