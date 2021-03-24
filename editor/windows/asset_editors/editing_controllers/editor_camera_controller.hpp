#pragma once

#include <ovis/core/transform.hpp>
#include <ovis/core/camera.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {
namespace editor {

class EditorCameraController : public SceneController {
 public:
  EditorCameraController(RenderingViewport* viewport);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  Camera camera_;
  Transform transform_;
  RenderingViewport* viewport_;
  bool right_button_down_ = false;
};

}  // namespace editor

}  // namespace ovis
