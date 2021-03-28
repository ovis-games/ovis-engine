#pragma once

#include <ovis/core/transform.hpp>
#include <ovis/core/camera.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {
namespace editor {

class EditorCameraController : public SceneController {
 public:
  EditorCameraController(Scene* game_scene);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  Camera camera_;
  Transform transform_;
  Scene* game_scene_;
};

}  // namespace editor

}  // namespace ovis
