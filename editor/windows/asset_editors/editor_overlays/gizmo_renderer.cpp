#include "gizmo_renderer.hpp"

#include "../editing_controllers/gizmo_controller.hpp"

#include <ovis/core/transform.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {
namespace editor {

GizmoRenderer::GizmoRenderer(Scene* editing_scene) : PrimitiveRenderer("GizmoRenderer"), editing_scene_(editing_scene) {
  RenderAfter("SpriteRenderer");
  SetDrawSpace(DrawSpace::SCREEN);
  enable_alpha_blending_ = true;
}

void GizmoRenderer::Render(const RenderContext& render_context) {
  auto* gizmo_controller = editing_scene_->GetController<GizmoController>("GizmoController");
  SDL_assert(gizmo_controller != nullptr);

  if (!gizmo_controller->object_selected_) {
    return;
  }

  BeginDraw(render_context);

  switch (gizmo_controller->gizmo_type()) {
    case GizmoController::GizmoType::MOVE:
      DrawArrow(Vector3::FromVector2(gizmo_controller->object_position_screen_space_),
                Vector3::FromVector2(gizmo_controller->x_axis_endpoint_screen_space_),
                gizmo_controller->selected_axes_ == GizmoController::AxisSelection::X ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                                                                                      : Color(1.0f, 0.0f, 0.0f, 0.6f),
                gizmo_controller->line_thickness_screen_space_);

      DrawArrow(Vector3::FromVector2(gizmo_controller->object_position_screen_space_),
                Vector3::FromVector2(gizmo_controller->y_axis_endpoint_screen_space_),
                gizmo_controller->selected_axes_ == GizmoController::AxisSelection::Y ? Color(0.0f, 1.0f, 0.0f, 1.0f)
                                                                                      : Color(0.0f, 1.0f, 0.0f, 0.6f),
                gizmo_controller->line_thickness_screen_space_);

      DrawArrow(Vector3::FromVector2(gizmo_controller->object_position_screen_space_),
                Vector3::FromVector2(gizmo_controller->z_axis_endpoint_screen_space_),
                gizmo_controller->selected_axes_ == GizmoController::AxisSelection::Z ? Color(0.0f, 0.0f, 1.0f, 1.0f)
                                                                                      : Color(0.0f, 0.0f, 1.0f, 0.6f),
                gizmo_controller->line_thickness_screen_space_);

      DrawDisc(Vector3::FromVector2(gizmo_controller->object_position_screen_space_),
               gizmo_controller->point_size_screen_space_,
               gizmo_controller->selected_axes_ == GizmoController::AxisSelection::XYZ ? Color(1.0f, 1.0f, 1.0f, 1.0f)
                                                                                       : Color(0.8f, 0.8f, 0.8f, 1.0f));
      break;

    case GizmoController::GizmoType::ROTATE: {
      const Vector3 x_direction = Vector3::FromVector2(gizmo_controller->x_axis_endpoint_screen_space_ -
                                                       gizmo_controller->object_position_screen_space_);
      const Vector3 y_direction = Vector3::FromVector2(gizmo_controller->y_axis_endpoint_screen_space_ -
                                                       gizmo_controller->object_position_screen_space_);
      const Vector3 z_direction = Vector3::FromVector2(gizmo_controller->z_axis_endpoint_screen_space_ -
                                                       gizmo_controller->object_position_screen_space_);

      DrawCircle(gizmo_controller->object_position_screen_space_, 1.0,
                 gizmo_controller->selected_axes_ == GizmoController::AxisSelection::X ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                                                                                       : Color(1.0f, 0.0f, 0.0f, 0.6f),
                 gizmo_controller->line_thickness_screen_space_, 400, z_direction, y_direction);

      DrawCircle(gizmo_controller->object_position_screen_space_, 1.0,
                 gizmo_controller->selected_axes_ == GizmoController::AxisSelection::Y ? Color(0.0f, 1.0f, 0.0f, 1.0f)
                                                                                       : Color(0.0f, 1.0f, 0.0f, 0.6f),
                 gizmo_controller->line_thickness_screen_space_, 400, x_direction, z_direction);

      DrawCircle(gizmo_controller->object_position_screen_space_, 1.0,
                 gizmo_controller->selected_axes_ == GizmoController::AxisSelection::Z ? Color(0.0f, 0.0f, 1.0f, 1.0f)
                                                                                       : Color(0.0f, 0.0f, 1.0f, 0.6f),
                 gizmo_controller->line_thickness_screen_space_, 400, x_direction, y_direction);
      break;
    }
  }

  EndDraw();
}

}  // namespace editor
}  // namespace ovis
