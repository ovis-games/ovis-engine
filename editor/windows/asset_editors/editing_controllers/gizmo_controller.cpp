#include "gizmo_controller.hpp"

namespace ovis {
namespace editor {

GizmoController::GizmoController(Scene* game_scene) : SceneController("GizmoController"), game_scene_(game_scene) {}

void GizmoController::ProcessEvent(Event* event) {}

}  // namespace editor
}  // namespace ovis
