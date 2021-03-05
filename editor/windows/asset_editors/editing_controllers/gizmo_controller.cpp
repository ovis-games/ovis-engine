#include "gizmo_controller.hpp"

#include <ovis/engine/input.hpp>

namespace ovis {
namespace editor {

GizmoController::GizmoController(Scene* game_scene) : SceneController("GizmoController"), game_scene_(game_scene) {
  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseWheelEvent::TYPE);
}

void GizmoController::ProcessEvent(Event* event) {}

}  // namespace editor
}  // namespace ovis
