#pragma once

#include "ovis/core/camera.hpp"
#include "ovis/core/scene_controller.hpp"
#include "ovis/core/transform.hpp"
#include "ovis/rendering/rendering_viewport.hpp"
#include "ovis/editor/viewport_controller.hpp"

namespace ovis {
namespace editor {

class EditorViewport;

class CameraController : public ViewportController {
 public:
  CameraController(EditorViewport* viewport);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  Camera camera_;
  Vector3 camera_position_;
};

}  // namespace editor
}  // namespace ovis
