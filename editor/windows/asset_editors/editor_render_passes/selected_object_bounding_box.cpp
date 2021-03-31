#include "selected_object_bounding_box.hpp"

#include "../editing_controllers/object_selection_controller.hpp"

#include <ovis/core/transform.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {
namespace editor {

SelectedObjectBoundingBox::SelectedObjectBoundingBox(Scene* editing_scene)
    : PrimitiveRenderer("SelectedObjectBoundingBox"), editing_scene_(editing_scene) {}

void SelectedObjectBoundingBox::Render(const RenderContext& render_context) {
  auto* object_selection_controller =
      editing_scene_->GetController<ObjectSelectionController>("ObjectSelectionController");
  SceneObject* scene_object = object_selection_controller->selected_object();

  if (scene_object != nullptr) {
    BeginDraw(render_context);

    Transform* transform = scene_object->GetComponent<Transform>("Transform");
    if (transform != nullptr) {
      const Vector3 clip_space_coordinates =
          TransformPosition(render_context.world_to_view_space, transform->position());
      const Vector3 screen_space_coordinates = viewport()->ClipSpacePositionToScreenSpace(clip_space_coordinates);
      DrawDisc(screen_space_coordinates, 20.0f, Color::Red());
      LogV("Drawing disc at {}", screen_space_coordinates);
    }

    EndDraw();
  }
}

}  // namespace editor
}  // namespace ovis
