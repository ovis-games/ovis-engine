#include "ovis/core/matrix.hpp"
#include <tuple>

#include <ovis/utils/log.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/core/virtual_machine.hpp>

namespace ovis {

namespace {
static const json SCHEMA = {{"$ref", "core#/$defs/transform"}};
}

json Transform::Serialize() const {
  return {{"position", local_position()},
          {"rotation", ExtractEulerAngles(local_rotation()) * RadiansToDegreesFactor<float>()},
          {"scale", local_scale()}};
}

bool Transform::Deserialize(const json& data) {
  if (data.contains("position")) {
    const Vector3 position = data.at("position");
    SetLocalPosition(position);
  }
  if (data.contains("rotation")) {
    const Vector3 euler_angles = Vector3(data.at("rotation")) * DegreesToRadiansFactor<float>();
    SetLocalRotation(Quaternion::FromEulerAngles(euler_angles.y, euler_angles.x, euler_angles.z));
  }
  if (data.contains("scale")) {
    const Vector3 scale = data.at("scale");
    SetLocalScale(scale);
  }
  // TODO: calculate matrices
  return true;
}

const json* Transform::GetSchema() const {
  return &SCHEMA;
}

void Transform::RegisterType(sol::table* module) {
  /// A class that respresents a 3D transformation of an object.
  // @classmod ovis.core.Transform
  // @base SceneObjectComponent
  // @usage local core = require "ovis.core"
  // local Transform = core.Transform
  sol::usertype<Transform> transform_type = module->new_usertype<Transform>("Transform", sol::no_constructor);

  /// The position of the transform in local space.
  // @field[type=Vector3] local_position
  transform_type["local_position"] = sol::property(&Transform::local_position, &Transform::SetLocalPosition);

  /// The position of the transform in world space.
  // @field[type=Vector3] world_position
  transform_type["world_position"] = sol::property(&Transform::world_position, &Transform::SetWorldPosition);

  /// The local scale of the transform.
  // @field[type=Vector3] local_scale
  transform_type["local_scale"] =
      sol::property(&Transform::local_scale, sol::resolve<void(Vector3)>(&Transform::SetLocalScale));

  /// The local rotation of the transform.
  // @field[type=Quaternion] local_rotation
  transform_type["local_rotation"] = sol::property(&Transform::local_rotation, &Transform::SetLocalRotation);

  /// Moves the transform using an offset on local space.
  // @function move
  // @see move
  // @param[type=Vector3] world_space_offset
  // @usage local transform = Transform.new()
  // assert(transform.position == core.Vector3.ZERO)
  // transform:move(core.Vector3.POSITIVE_X)
  // assert(transform.position == core.Vector3.POSITIVE_X)
  // transform:move(core.Vector3.POSITIVE_X)
  // assert(transform.position == 2 * core.Vector3.POSITIVE_X)
  transform_type["move_locally"] = &Transform::MoveLocally;

  /// Moves the transform using an offset on world space.
  // @function move
  // @see move_locally
  // @param[type=Vector3] world_space_offset
  // @usage local transform = Transform.new()
  // assert(transform.position == core.Vector3.ZERO)
  // transform:move(core.Vector3.POSITIVE_X)
  // assert(transform.position == core.Vector3.POSITIVE_X)
  // transform:move(core.Vector3.POSITIVE_X)
  // assert(transform.position == 2 * core.Vector3.POSITIVE_X)
  transform_type["move"] = &Transform::Move;

  /// Rotates the object by multiplying its rotation by another quaternion.
  // @function rotate
  // @param[type=Quaternion] rotation_offset

  /// Rotates the object by rotating it around an in local space axis.
  // The angle is given in degrees.
  // @function rotate
  // @param[type=Vector3] axis
  // @param[type=number] angle
  transform_type["rotate_locally"] = sol::overload(sol::resolve<void(Quaternion)>(&Transform::RotateLocally),
                                                   sol::resolve<void(Vector3, float)>(&Transform::RotateLocally));

  /// Sets the rotation to the given yaw, pitch and roll in local space.
  // All angles are given in degrees.
  // @function set_local_yaw_pitch_roll
  // @tparam number yaw
  // @tparam number pitch
  // @tparam number roll
  // @usage local transform = Transform.new()
  // transform:set_local_yaw_pitch_roll(45, 90, 0)
  transform_type["set_yaw_pitch_roll"] = &Transform::SetLocalYawPitchRoll;

  /// Returns the yaw, pitch and roll of the rotation in local space.
  // All angles are given in degrees.
  // @function get_local_yaw_pitch_roll
  // @treturn number yaw
  // @treturn number pitch
  // @treturn number roll
  // @usage local transform = Transform.new()
  // local yaw, pitch, roll = transform:get_local_yaw_pitch_roll()
  // assert(yaw == 0)
  // assert(pitch == 0)
  // assert(roll == 0)
  transform_type["get_local_yaw_pitch_roll"] = [](const Transform* transform) {
    std::tuple<float, float, float> yaw_pitch_roll;
    transform->GetLocalYawPitchRoll(&std::get<0>(yaw_pitch_roll), &std::get<1>(yaw_pitch_roll),
                                    &std::get<2>(yaw_pitch_roll));
    return yaw_pitch_roll;
  };

  /// Transforms a direcion from object to world space.
  // If the transform contains any scaling the magnitude of the vector will likely change.
  // @function local_direction_to_world
  // @param[type=Vector3] direction
  // @treturn Vector3
  // @see 03-spaces.md
  // @see world_direction_to_object_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:rotate(Vector3.POSITIVE_Y, 90)
  // local transformed_direction = transform:local_direction_to_world(Vector3.POSITIVE_X)
  // assert(Vector3.length(transformed_direction - Vector3.POSITIVE_Z) < 0.1)
  transform_type["local_direction_to_world"] = &Transform::LocalDirectionToWorld;

  /// Transforms a direcion from world to local space.
  // If the transform contains any scaling the magnitude of the vector will likely change.
  // @function world_direction_to_local
  // @param[type=Vector3] direction
  // @treturn Vector3
  // @see 03-spaces.md
  // @see object_direction_to_world_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:rotate(Vector3.POSITIVE_Y, 90)
  // local transformed_direction = transform:world_direction_to_local(Vector3.POSITIVE_Z)
  // assert(Vector3.length(transformed_direction - Vector3.POSITIVE_X) < 0.1)
  transform_type["world_direction_to_local"] = &Transform::WorldDirectionToLocal;

  /// Transforms a position in local space to world space.
  // @function local_position_to_world
  // @param[type=Vector3] object_space_coordinates
  // @treturn Vector3
  // @see 03-spaces.md
  // @see world_position_to_local_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:move(Vector3.new(0, 1, 0))
  // local transformed_position = transform:local_position_to_world(Vector3.new(1, 0, 0))
  // assert(transformed_position == Vector3.new(1, 1, 0))
  transform_type["local_position_to_world"] = &Transform::LocalPositionToWorld;

  /// Transforms a position in  world space to local space.
  // @function world_position_to_local
  // @param[type=Vector3] world_space_coordinates
  // @treturn Vector3
  // @see 03-spaces.md
  // @see local_position_to_world_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:move(Vector3.new(0, 1, 0))
  // local transformed_position = transform:world_position_to_local(Vector3.new(1, 1, 0))
  // assert(transformed_position == Vector3.new(1, 0, 0))
  transform_type["world_position_to_local"] = &Transform::WorldPositionToLocal;
}

void Transform::RegisterType(vm::Module* module) {
  auto transform_type = module->RegisterType<Transform, SceneObjectComponent>("Transform");
  // transform_type->RegisterProperty<&Transform::local_position, &Transform::SetLocalPosition>("Local Position");
  
  // auto transform_constructor = module->RegisterFunction<vm::Constructor<Transform, SceneObject*>>("Create Transform", { "object" }, { "transform" });
}

void Transform::CalculateMatrices() const {
  const Matrix3x4 local_to_parent = Matrix3x4::FromTransformation(position_, scale_, rotation_);
  local_to_world_ = AffineCombine(FindParentToWorldMatrix(), local_to_parent);
  world_to_local_ = InvertAffine(local_to_world_);
  dirty_ = false;
}

void Transform::FlagAsDirty() {
  dirty_ = true;
  scene_object()->ForEachChild(true, static_cast<void (*)(SceneObject*)>(FlagAsDirty));
}

void Transform::FlagAsDirty(SceneObject* object) {
  if (object->HasComponent<Transform>()) {
    object->GetComponent<Transform>()->dirty_ = true;
  }
}

}  // namespace ovis
