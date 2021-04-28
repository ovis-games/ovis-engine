#include "object_selection_controller.hpp"

#include <ovis/utils/log.hpp>
#include <ovis/core/intersection.hpp>
#include <ovis/rendering2d/sprite.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

namespace {

AxisAlignedBoundingBox3D GetComponentAABB(std::string_view id, SceneObjectComponent* component) {
  if (id == "Sprite") {
    Sprite* sprite = down_cast<Sprite*>(component);
    return AxisAlignedBoundingBox3D::FromCenterAndExtend(Vector3::Zero(), Vector3::FromVector2(sprite->size()));
  } else {
    return AxisAlignedBoundingBox3D::Empty();
  }
};

}  // namespace

ObjectSelectionController::ObjectSelectionController() : EditorController(Name()) {
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
}

void ObjectSelectionController::Update(std::chrono::microseconds) {
  SceneObject* object = selected_object();
  if (!object) {
    return;
  }

  selected_object_aabb_ = AxisAlignedBoundingBox3D::Empty();
  for (const std::string& component_id : object->GetComponentIds()) {
    const auto aabb = GetComponentAABB(component_id, object->GetComponent(component_id));
    selected_object_aabb_ = AxisAlignedBoundingBox3D::FromMinMax(min(aabb.min(), selected_object_aabb_.min()),
                                                                 max(aabb.max(), selected_object_aabb_.max()));
  }
}

void ObjectSelectionController::ProcessEvent(Event* event) {
  if (event->type() == MouseButtonPressEvent::TYPE) {
    auto* mouse_button_press_event = down_cast<MouseButtonPressEvent*>(event);
    const Ray3D view_ray_world_space =
        mouse_button_press_event->viewport()->CalculateViewRay(mouse_button_press_event->screen_space_position());

    float closest_distance = Infinity<float>();
    SceneObject* closest_object = nullptr;

    for (SceneObject* object : game_scene()->GetObjects()) {
      const Transform* transform = object->GetComponent<Transform>("Transform");
      const Matrix3x4 world_to_object_space =
          transform ? transform->world_to_local_matrix() : Matrix3x4::IdentityTransformation();
      const Matrix3x4 object_to_world_space =
          transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();
      const Ray3D view_ray_object_space{
          TransformPosition(world_to_object_space, view_ray_world_space.origin),
          Normalize(TransformDirection(world_to_object_space, view_ray_world_space.direction))};
      for (const std::string& component_id : object->GetComponentIds()) {
        const auto aabb = GetComponentAABB(component_id, object->GetComponent(component_id));
        const auto intersection = ComputeRayAABBIntersection(view_ray_object_space, aabb, 0.0f, closest_distance);
        if (intersection) {
          closest_distance = intersection->t_ray_enter;
          closest_object = object;
        }
      }
    }

    if (closest_object != nullptr) {
      SelectObject(closest_object);
      mouse_button_press_event->StopPropagation();
    }
  }
}

void ObjectSelectionController::SelectObject(const std::string& object_name) {
  LogV("Selected object: {}", object_name);
  selected_object_name_ = object_name;
  CheckSelectionValidity();
}
void ObjectSelectionController::SelectObject(SceneObject* object) {
  if (object != nullptr) {
    SDL_assert(object->scene() == game_scene());
    SelectObject(object->name());
  } else {
    selected_object_name_ = "";
  }
}
void ObjectSelectionController::ClearSelection() {
  LogV("Cleared selection");
  selected_object_name_ = "";
}

bool ObjectSelectionController::has_selected_object() const {
  CheckSelectionValidity();
  return selected_object_name_ != "";
}
const std::string& ObjectSelectionController::selected_object_name() const {
  CheckSelectionValidity();
  return selected_object_name_;
}
SceneObject* ObjectSelectionController::selected_object() const {
  CheckSelectionValidity();
  return selected_object_name_.length() > 0 ? game_scene()->GetObject(selected_object_name_) : nullptr;
}

void ObjectSelectionController::CheckSelectionValidity() const {
  if (selected_object_name_ != "" && !game_scene()->ContainsObject(selected_object_name_)) {
    LogV("The game scene does not contain the object {}", selected_object_name_);
    selected_object_name_ = "";
  }
}

SceneObject* GetSelectedObject(Scene* editing_scene) {
  auto* object_selection_controller = editing_scene->GetController<ObjectSelectionController>();
  if (object_selection_controller == nullptr) {
    return nullptr;
  } else {
    return object_selection_controller->selected_object();
  }
}

}  // namespace editor
}  // namespace ovis
