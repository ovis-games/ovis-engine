#include "gizmo_renderer.hpp"

#include "../editing_controllers/object_selection_controller.hpp"

#include <ovis/base/transform_component.hpp>

#include <ovis/engine/viewport.hpp>

namespace ovis {
namespace editor {

GizmoRenderer::GizmoRenderer(Scene* editing_scene) : DebugRenderPass("GizmoRenderer"), editing_scene_(editing_scene) {}

void GizmoRenderer::Render(const RenderContext& render_context) {
  auto* object_selection_controller =
      editing_scene_->GetController<ObjectSelectionController>("ObjectSelectionController");
  SceneObject* scene_object = object_selection_controller->selected_object();

  if (scene_object != nullptr) {
    BeginDraw(render_context);

    TransformComponent* transform = scene_object->GetComponent<TransformComponent>("Transform");
    if (transform != nullptr) {
      vector3 x_direction = transform->TransformDirection(vector3(1.0f, 0.0f, 0.0f));
      DrawLine(transform->translation(), transform->translation() + gizmo_radius_ * x_direction,
               color(1.0f, 0.0f, 0.0f, 1.0f));
    }

    EndDraw();
  }
}

}  // namespace editor
}  // namespace ovis
