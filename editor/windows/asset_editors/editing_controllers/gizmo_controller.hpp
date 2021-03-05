#pragma once

#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>

namespace ovis {
namespace editor {

class GizmoController : public SceneController {
 public:
  enum class GizmoType { MOVE, ROTATE, SCALE };

  GizmoController(Scene* game_scene);

  void ProcessEvent(Event* event) override;

  inline void SetGizmoType(GizmoType type) { type_ = type; }
  inline GizmoType gizmo_type() { return type_; }

 private:
  Scene* game_scene_;
  GizmoType type_;
};

}  // namespace editor
}  // namespace ovis
