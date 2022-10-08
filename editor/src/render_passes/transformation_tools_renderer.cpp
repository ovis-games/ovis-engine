#include "ovis/editor/render_passes/transformation_tools_renderer.hpp"

#include "ovis/core/transform.hpp"
#include "ovis/rendering/rendering_viewport.hpp"
#include "ovis/rendering2d/renderer2d.hpp"
#include "ovis/editor/editor_viewport.hpp"

namespace ovis {
namespace editor {

TransformationToolsRenderer::TransformationToolsRenderer() {
  RenderAfter<Renderer2D>();
  SetDrawSpace(DrawSpace::SCREEN);
  enable_alpha_blending_ = true;
}

void TransformationToolsRenderer::Render(const RenderContext& render_context) {
  // auto* controller = EditorViewport::instance()->transformation_tools_controller();
  // SDL_assert(controller != nullptr);

  // if (!controller->object_selected_) {
  //   return;
  // }

  // BeginDraw(render_context);

  // switch (controller->transformation_type()) {
  //   case TransformationToolsController::TransformationType::MOVE:
  //     DrawArrow(Vector3::FromVector2(controller->object_position_screen_space_),
  //               Vector3::FromVector2(controller->x_axis_endpoint_screen_space_),
  //               controller->selected_axes_ == TransformationToolsController::AxesSelection::X
  //                   ? Color(1.0f, 0.0f, 0.0f, 1.0f)
  //                   : Color(1.0f, 0.0f, 0.0f, 0.6f),
  //               controller->line_thickness_screen_space_);

  //     DrawArrow(Vector3::FromVector2(controller->object_position_screen_space_),
  //               Vector3::FromVector2(controller->y_axis_endpoint_screen_space_),
  //               controller->selected_axes_ == TransformationToolsController::AxesSelection::Y
  //                   ? Color(0.0f, 1.0f, 0.0f, 1.0f)
  //                   : Color(0.0f, 1.0f, 0.0f, 0.6f),
  //               controller->line_thickness_screen_space_);

  //     DrawArrow(Vector3::FromVector2(controller->object_position_screen_space_),
  //               Vector3::FromVector2(controller->z_axis_endpoint_screen_space_),
  //               controller->selected_axes_ == TransformationToolsController::AxesSelection::Z
  //                   ? Color(0.0f, 0.0f, 1.0f, 1.0f)
  //                   : Color(0.0f, 0.0f, 1.0f, 0.6f),
  //               controller->line_thickness_screen_space_);

  //     DrawDisc(Vector3::FromVector2(controller->object_position_screen_space_), controller->point_size_screen_space_,
  //              controller->selected_axes_ == TransformationToolsController::AxesSelection::XYZ
  //                  ? Color(1.0f, 1.0f, 1.0f, 1.0f)
  //                  : Color(0.8f, 0.8f, 0.8f, 1.0f));
  //     break;

  //   case TransformationToolsController::TransformationType::ROTATE: {
  //     const Vector3 x_direction =
  //         Vector3::FromVector2(controller->x_axis_endpoint_screen_space_ - controller->object_position_screen_space_);
  //     const Vector3 y_direction =
  //         Vector3::FromVector2(controller->y_axis_endpoint_screen_space_ - controller->object_position_screen_space_);
  //     const Vector3 z_direction =
  //         Vector3::FromVector2(controller->z_axis_endpoint_screen_space_ - controller->object_position_screen_space_);

  //     DrawCircle(controller->object_position_screen_space_, 1.0,
  //                controller->selected_axes_ == TransformationToolsController::AxesSelection::X
  //                    ? Color(1.0f, 0.0f, 0.0f, 1.0f)
  //                    : Color(1.0f, 0.0f, 0.0f, 0.6f),
  //                controller->line_thickness_screen_space_, 400, z_direction, y_direction);

  //     DrawCircle(controller->object_position_screen_space_, 1.0,
  //                controller->selected_axes_ == TransformationToolsController::AxesSelection::Y
  //                    ? Color(0.0f, 1.0f, 0.0f, 1.0f)
  //                    : Color(0.0f, 1.0f, 0.0f, 0.6f),
  //                controller->line_thickness_screen_space_, 400, x_direction, z_direction);

  //     DrawCircle(controller->object_position_screen_space_, 1.0,
  //                controller->selected_axes_ == TransformationToolsController::AxesSelection::Z
  //                    ? Color(0.0f, 0.0f, 1.0f, 1.0f)
  //                    : Color(0.0f, 0.0f, 1.0f, 0.6f),
  //                controller->line_thickness_screen_space_, 400, x_direction, y_direction);
  //     break;
  //   }

  //   case TransformationToolsController::TransformationType::SCALE:
  //     DrawLine(Vector3::FromVector2(controller->object_position_screen_space_),
  //              Vector3::FromVector2(controller->x_axis_endpoint_screen_space_),
  //              controller->selected_axes_ == TransformationToolsController::AxesSelection::X
  //                  ? Color(1.0f, 0.0f, 0.0f, 1.0f)
  //                  : Color(1.0f, 0.0f, 0.0f, 0.6f),
  //              controller->line_thickness_screen_space_);
  //     DrawPoint(Vector3::FromVector2(controller->x_axis_endpoint_screen_space_),
  //               controller->line_thickness_screen_space_ * 2,
  //               controller->selected_axes_ == TransformationToolsController::AxesSelection::X
  //                   ? Color(1.0f, 0.0f, 0.0f, 1.0f)
  //                   : Color(1.0f, 0.0f, 0.0f, 0.6f));

  //     DrawLine(Vector3::FromVector2(controller->object_position_screen_space_),
  //              Vector3::FromVector2(controller->y_axis_endpoint_screen_space_),
  //              controller->selected_axes_ == TransformationToolsController::AxesSelection::Y
  //                  ? Color(0.0f, 1.0f, 0.0f, 1.0f)
  //                  : Color(0.0f, 1.0f, 0.0f, 0.6f),
  //              controller->line_thickness_screen_space_);
  //     DrawPoint(Vector3::FromVector2(controller->y_axis_endpoint_screen_space_),
  //               controller->line_thickness_screen_space_ * 2,
  //               controller->selected_axes_ == TransformationToolsController::AxesSelection::Y
  //                   ? Color(0.0f, 1.0f, 0.0f, 1.0f)
  //                   : Color(0.0f, 1.0f, 0.0f, 0.6f));

  //     DrawLine(Vector3::FromVector2(controller->object_position_screen_space_),
  //              Vector3::FromVector2(controller->z_axis_endpoint_screen_space_),
  //              controller->selected_axes_ == TransformationToolsController::AxesSelection::Z
  //                  ? Color(0.0f, 0.0f, 1.0f, 1.0f)
  //                  : Color(0.0f, 0.0f, 1.0f, 0.6f),
  //              controller->line_thickness_screen_space_);
  //     DrawPoint(Vector3::FromVector2(controller->z_axis_endpoint_screen_space_),
  //               controller->line_thickness_screen_space_ * 2,
  //               controller->selected_axes_ == TransformationToolsController::AxesSelection::Z
  //                   ? Color(0.0f, 0.0f, 1.0f, 1.0f)
  //                   : Color(0.0f, 0.0f, 1.0f, 0.6f));

  //     DrawDisc(Vector3::FromVector2(controller->object_position_screen_space_), controller->point_size_screen_space_,
  //              controller->selected_axes_ == TransformationToolsController::AxesSelection::XYZ
  //                  ? Color(1.0f, 1.0f, 1.0f, 1.0f)
  //                  : Color(0.8f, 0.8f, 0.8f, 1.0f));
  //     break;
  // }

  // EndDraw();
}

}  // namespace editor
}  // namespace ovis
