#include <ovis/scene/scene.hpp>
#include <ovis/scene/transform.hpp>
#include <ovis/scene/transform_controller.hpp>

namespace ovis {

TransformController::TransformController() : SceneController("TransformController") {}

void TransformController::Update(std::chrono::microseconds delta_time) {
  for (auto* object : scene()->GetSceneObjectsWithComponent("Transform")) {
    Transform* transform = object->GetComponent<Transform>("Transform");
    SDL_assert(transform);
    transform->local_to_world_ =
        Matrix3x4::FromTransformation(transform->position(), transform->scale(), transform->rotation());
    transform->world_to_local_ =
        Matrix3x4::FromTransformation(-transform->position(), 1.0f / transform->scale(), Invert(transform->rotation()));
  }
}

void TransformController::RegisterType(sol::table* module) {
  /// Applies the changes in the transformation component.
  // All controllers that change the transformation of an object should run before
  // this controllers because this controller updates the transformation matrices
  // which are used to calculate the global transformation of the object.
  // @classmod ovis.scene.TransformController
  // @base SceneController
  sol::usertype<TransformController> scene_controller_type = module->new_usertype<TransformController>("TransformController");

  /// The scene, the controller is attached to.
  // @field[type=Scene] scene
  scene_controller_type["scene"] = sol::property(&TransformController::scene);
  
  /// The name of the controller.
  // @field[type=string] name
  scene_controller_type["name"] = sol::property(&TransformController::name);
}


}  // namespace ovis
