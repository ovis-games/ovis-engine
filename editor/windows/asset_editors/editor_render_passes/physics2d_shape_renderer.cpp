#include "physics2d_shape_renderer.hpp"

#include "../editing_controllers/object_selection_controller.hpp"
#include "../editing_controllers/physics2d_shape_controller.hpp"
#include "transformation_tools_renderer.hpp"

#include <box2d/b2_chain_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <box2d/b2_polygon_shape.h>

#include <ovis/core/transform.hpp>
#include <ovis/rendering/rendering_viewport.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {
namespace editor {

Physics2DShapeRenderer::Physics2DShapeRenderer(Scene* editing_scene)
    : PrimitiveRenderer(Name()), editing_scene_(editing_scene) {
  enable_alpha_blending_ = true;
  RenderAfter(std::string(TransformationToolsRenderer::Name()));
}

void Physics2DShapeRenderer::Render(const RenderContext& render_context) {
  if (viewport()->scene()->is_playing()) {
    return;
  }

  auto* object_selection_controller = editing_scene_->GetController<ObjectSelectionController>();
  SceneObject* object = object_selection_controller->selected_object();
  if (object == nullptr) {
    return;
  }

  const RigidBody2DFixture* rigid_body_fixture = object->GetComponent<RigidBody2DFixture>("RigidBody2DFixture");
  if (!rigid_body_fixture) {
    return;
  }

  const b2Shape* shape = rigid_body_fixture->shape();
  BeginDraw(render_context);

  Transform* transform = object->GetComponent<Transform>("Transform");
  const Matrix3x4 local_to_world = transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();
  const Vector3 origin = TransformPosition(local_to_world, Vector3::Zero());
  const Color color_opaque = Color::Aqua();
  const Color color_transparent = Color(color_opaque.r, color_opaque.g, color_opaque.b, 0.5f);
  const Color color_selected = Color::Yellow();
  const Color color_error = Color::Red();

  auto* controller = editing_scene_->GetController<Physics2DShapeController>();
  SDL_assert(controller != nullptr);

  std::vector<Vector3> world_space_vertices;
  std::transform(controller->vertices_.begin(), controller->vertices_.end(), std::back_inserter(world_space_vertices),
                 [&](const Vector2& vertex) {
                   return TransformPosition(controller->body_to_world_, Vector3::FromVector2(vertex));
                 });

  switch (controller->type_) {
    case b2Shape::e_circle: {
      DrawDisc(origin, controller->radius_, color_transparent);
      const Color color = controller->selection_ == 0 ? color_selected : color_opaque;
      DrawCircle(origin, controller->radius_, color);
      break;
    }

    case b2Shape::e_polygon: {
      DrawConvexPolygon(world_space_vertices.data(), world_space_vertices.size(), color_transparent);
      for (const auto& vertex : IndexRange(world_space_vertices)) {
        const Color color = controller->selection_ == vertex.index() ? color_selected : color_opaque;
        DrawPoint(vertex.value(), 10.0f, color);
      }
      if (controller->error_vertex_.has_value()) {
        DrawPoint(TransformPosition(controller->body_to_world_, Vector3::FromVector2(*controller->error_vertex_)),
                  10.0f, color_error);
      }
      break;
    }

    case b2Shape::e_edge: {
      SDL_assert(world_space_vertices.size() == 4);
      DrawLine(world_space_vertices[1], world_space_vertices[2], color_transparent);

      if (controller->one_sided_edge_) {
        DrawDashedLine(world_space_vertices[1], world_space_vertices[0], color_transparent);
        DrawDashedLine(world_space_vertices[2], world_space_vertices[3], color_transparent);

        DrawPoint(world_space_vertices[0], 10.0f, controller->selection_ == 0 ? color_selected : color_opaque);
        DrawPoint(world_space_vertices[3], 10.0f, controller->selection_ == 3 ? color_selected : color_opaque);
      }

      DrawPoint(world_space_vertices[1], 10.0f, controller->selection_ == 1 ? color_selected : color_opaque);
      DrawPoint(world_space_vertices[2], 10.0f, controller->selection_ == 2 ? color_selected : color_opaque);
      break;
    }

    case b2Shape::e_chain: {
      SDL_assert(world_space_vertices.size() >= 4);
      DrawLineStip(std::span<Vector3>(world_space_vertices.data() + 1, world_space_vertices.size() - 2),
                   color_transparent);
      DrawDashedLine(world_space_vertices[1], world_space_vertices[0], color_transparent);
      DrawDashedLine(world_space_vertices[world_space_vertices.size() - 2],
                     world_space_vertices[world_space_vertices.size() - 1], color_transparent);
      for (const auto& vertex : IndexRange(world_space_vertices)) {
        const Color color = controller->selection_ == vertex.index() ? color_selected : color_opaque;
        DrawPoint(vertex.value(), 10.0f, color);
      }
      break;
    }

    case b2Shape::e_typeCount:
      SDL_assert(false);
      break;
  }

  EndDraw();
}

}  // namespace editor
}  // namespace ovis
