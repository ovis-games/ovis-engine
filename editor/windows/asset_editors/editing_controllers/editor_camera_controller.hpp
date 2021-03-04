#pragma once

#include <ovis/math/transform.hpp>
#include <ovis/engine/camera.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/viewport.hpp>

namespace ovis {
namespace editor {

class EditorCameraController : public SceneController {
 public:
  EditorCameraController(Viewport* viewport);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  Camera camera_;
  Transform transform_;
  Viewport* viewport_;
  bool right_button_down_ = false;
};

}  // namespace editor

}  // namespace ovis
