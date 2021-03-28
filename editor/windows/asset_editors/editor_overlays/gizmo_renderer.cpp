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
      DrawArrow(Vector3::FromVector2(gizmo_controller->line_x_.endpoints[0]),
                Vector3::FromVector2(gizmo_controller->line_x_.endpoints[1]),
                gizmo_controller->movement_selection_ == GizmoController::MovementSelection::X
                    ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                    : Color(1.0f, 0.0f, 0.0f, 0.6f),
                gizmo_controller->line_thickness_);

      DrawArrow(Vector3::FromVector2(gizmo_controller->line_y_.endpoints[0]),
                Vector3::FromVector2(gizmo_controller->line_y_.endpoints[1]),
                gizmo_controller->movement_selection_ == GizmoController::MovementSelection::Y
                    ? Color(0.0f, 1.0f, 0.0f, 1.0f)
                    : Color(0.0f, 1.0f, 0.0f, 0.6f),
                gizmo_controller->line_thickness_);

      DrawArrow(Vector3::FromVector2(gizmo_controller->line_z_.endpoints[0]),
                Vector3::FromVector2(gizmo_controller->line_z_.endpoints[1]),
                gizmo_controller->movement_selection_ == GizmoController::MovementSelection::Z
                    ? Color(0.0f, 0.0f, 1.0f, 1.0f)
                    : Color(0.0f, 0.0f, 1.0f, 0.6f),
                gizmo_controller->line_thickness_);

      DrawDisc(Vector3::FromVector2(gizmo_controller->object_position_), gizmo_controller->point_size_,
               gizmo_controller->movement_selection_ == GizmoController::MovementSelection::XYZ
                   ? Color(1.0f, 1.0f, 1.0f, 1.0f)
                   : Color(0.8f, 0.8f, 0.8f, 1.0f));
      break;

    case GizmoController::GizmoType::ROTATE: {
      const Vector3 x_direction =
          Vector3::FromVector2(gizmo_controller->line_x_.endpoints[1] - gizmo_controller->line_x_.endpoints[0]);
      const Vector3 y_direction =
          Vector3::FromVector2(gizmo_controller->line_y_.endpoints[1] - gizmo_controller->line_y_.endpoints[0]);
      const Vector3 z_direction =
          Vector3::FromVector2(gizmo_controller->line_z_.endpoints[1] - gizmo_controller->line_z_.endpoints[0]);

      DrawCircle(Vector3::FromVector2(gizmo_controller->object_position_), 1.0f,
                 gizmo_controller->movement_selection_ == GizmoController::MovementSelection::X
                     ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                     : Color(1.0f, 0.0f, 0.0f, 0.6f),
                 gizmo_controller->line_thickness_, 400, z_direction, y_direction);

      DrawCircle(Vector3::FromVector2(gizmo_controller->object_position_), 1.0f,
                 gizmo_controller->movement_selection_ == GizmoController::MovementSelection::Y
                     ? Color(0.0f, 1.0f, 0.0f, 1.0f)
                     : Color(0.0f, 1.0f, 0.0f, 0.6f),
                 gizmo_controller->line_thickness_, 400, x_direction, z_direction);

      DrawCircle(Vector3::FromVector2(gizmo_controller->object_position_), 1.0f,
                 gizmo_controller->movement_selection_ == GizmoController::MovementSelection::Z
                     ? Color(0.0f, 0.0f, 1.0f, 1.0f)
                     : Color(0.0f, 0.0f, 1.0f, 0.6f),
                 gizmo_controller->line_thickness_, 400, x_direction, y_direction);
      break;
    }
  }

  EndDraw();
}

}  // namespace editor
}  // namespace ovis
