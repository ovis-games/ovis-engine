#include "gizmo_renderer.hpp"

#include "../editing_controllers/object_selection_controller.hpp"

#include <ovis/core/transform.hpp>

#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {
namespace editor {

GizmoRenderer::GizmoRenderer(Scene* editing_scene) : DebugRenderPass("GizmoRenderer"), editing_scene_(editing_scene) {}

void GizmoRenderer::Render(const RenderContext& render_context) {
  auto* object_selection_controller =
      editing_scene_->GetController<ObjectSelectionController>("ObjectSelectionController");
  SceneObject* scene_object = object_selection_controller->selected_object();

  if (scene_object != nullptr) {
    BeginDraw(render_context);

    Transform* transform = scene_object->GetComponent<Transform>("Transform");
    if (transform != nullptr) {
      Vector3 x_direction = transform->LocalDirectionToWorldSpace(Vector3::PositiveX());
      DrawLine(transform->position(), transform->position() + gizmo_radius_ * x_direction, Color::Red());
    }

    EndDraw();
  }
}

}  // namespace editor
}  // namespace ovis
