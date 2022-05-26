#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <ovis/editor_viewport/editor_viewport.hpp>
#include <ovis/editor_viewport/object_selection_controller.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/intersection.hpp>
#include <ovis/core/main_vm.hpp>
#include <ovis/rendering2d/shape2d.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

namespace {

AxisAlignedBoundingBox3D GetComponentAABB(const Value& component) {
  if (component.type_id() == main_vm->GetTypeId<Shape2D>()) {
    Shape2D* shape = component.as<Shape2D*>();
    switch (shape->type()) {
      case Shape2D::Type::RECTANGLE:
        return AxisAlignedBoundingBox3D::FromCenterAndExtend(Vector3::Zero(),
                                                             Vector3::FromVector2(shape->rectangle().size));

      case Shape2D::Type::ELLIPSE:
        return AxisAlignedBoundingBox3D::FromCenterAndExtend(Vector3::Zero(),
                                                             Vector3::FromVector2(shape->ellipse().size));
    }
  } else {
    return AxisAlignedBoundingBox3D::Empty();
  }
};

void SelectObject(std::string path) {
  EditorViewport::instance()->object_selection_controller()->SelectObject(path);
}

EMSCRIPTEN_BINDINGS(editor_viewport_module) {
  using namespace emscripten;
  function("viewportSelectObject", &SelectObject);
}

}  // namespace

ObjectSelectionController::ObjectSelectionController() {
}

void ObjectSelectionController::Update(std::chrono::microseconds) {
  SceneObject* object = selected_object();
  if (!object) {
    return;
  }

  selected_object_aabb_ = AxisAlignedBoundingBox3D::Empty();
  for (const auto component_type : object->component_type_ids()) {
    auto component = object->GetComponent(component_type);
    if (!component) {
      continue;
    }
    const auto aabb = GetComponentAABB(*component);
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

    for (SceneObject* object : viewport()->scene()->objects()) {
      const Transform* transform = object->GetComponent<Transform>();
      const Matrix3x4 world_to_object_space =
          transform ? transform->world_to_local_matrix() : Matrix3x4::IdentityTransformation();
      const Matrix3x4 object_to_world_space =
          transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();
      const Ray3D view_ray_object_space{
          TransformPosition(world_to_object_space, view_ray_world_space.origin),
          Normalize(TransformDirection(world_to_object_space, view_ray_world_space.direction))};
      for (const auto component_type : object->component_type_ids()) {
        auto component = object->GetComponent(component_type);
        if (!component) {
          continue;
        }
        const auto aabb = GetComponentAABB(*component);
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

void ObjectSelectionController::SelectObject(SceneObject* object) {
  auto selection_event = emscripten::val::object();
  selection_event.set("type", "object_selection");
  if (object != nullptr) {
    selected_object_path_ = object->path();
    selection_event.set("path", *selected_object_path_);
  } else {
    selected_object_path_.reset();
  }
  viewport()->SendEvent(selection_event);
}

void ObjectSelectionController::SelectObject(std::string_view object_path) {
  SelectObject(viewport()->scene()->GetObject(object_path));
}

bool ObjectSelectionController::has_selected_object() const {
  return selected_object_path_.has_value() && viewport()->scene()->ContainsObject(*selected_object_path_);
}

SceneObject* ObjectSelectionController::selected_object() const {
  return selected_object_path_.has_value() ? viewport()->scene()->GetObject(*selected_object_path_) : nullptr;
}

}  // namespace editor
}  // namespace ovis
