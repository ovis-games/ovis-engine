#include <tuple>

#include <ovis/utils/log.hpp>
#include <ovis/core/transform.hpp>

namespace ovis {

namespace {
static const json SCHEMA = {{"$ref", "core#/$defs/transform"}};
}

json Transform::Serialize() const {
  return {{"position", position()},
          {"rotation", ExtractEulerAngles(rotation()) * RadiansToDegreesFactor<float>()},
          {"scale", scale()}};
}

bool Transform::Deserialize(const json& data) {
  if (data.contains("position")) {
    const Vector3 position = data.at("position");
    SetPosition(position);
  }
  if (data.contains("rotation")) {
    const Vector3 euler_angles = Vector3(data.at("rotation")) * DegreesToRadiansFactor<float>();
    SetRotation(Quaternion::FromEulerAngles(euler_angles.y, euler_angles.x, euler_angles.z));
  }
  if (data.contains("scale")) {
    const Vector3 scale = data.at("scale");
    SetScale(scale);
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
  sol::usertype<Transform> transform_type =
      module->new_usertype<Transform>("Transform", sol::no_constructor);

  /// The position of the transformation.
  // @field[type=Vector3] position
  transform_type["position"] = sol::property(&Transform::position, &Transform::SetPosition);

  /// The scale of the transformation.
  // @field[type=Vector3] scale
  transform_type["scale"] = sol::property(&Transform::scale, sol::resolve<void(Vector3)>(&Transform::SetScale));

  /// The rotation of the transformation.
  // @field[type=Quaternion] rotation
  transform_type["rotation"] = sol::property(&Transform::rotation, &Transform::SetRotation);

  /// Moves the transformation.
  // @function move
  // @param[type=Vector3] offset
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

  /// Rotates the object by rotating it around an axis.
  // The angle is given in degrees.
  // @function rotate
  // @param[type=Vector3] axis
  // @param[type=number] angle
  transform_type["rotate"] = sol::overload(sol::resolve<void(Quaternion)>(&Transform::Rotate),
                                           sol::resolve<void(Vector3, float)>(&Transform::Rotate));

  /// Sets the rotation to the given yaw, pitch and roll.
  // All angles are given in degrees.
  // @function set_yaw_pitch_roll
  // @tparam number yaw
  // @tparam number pitch
  // @tparam number roll
  // @usage local transform = Transform.new()
  // transform:set_yaw_pitch_roll(45, 90, 0)
  transform_type["set_yaw_pitch_roll"] = &Transform::SetYawPitchRoll;

  /// Returns the yaw, pitch and roll of the rotation.
  // All angles are given in degrees.
  // @function get_yaw_pitch_roll
  // @treturn number yaw
  // @treturn number pitch
  // @treturn number roll
  // @usage local transform = Transform.new()
  // local yaw, pitch, roll = transform:get_yaw_pitch_roll()
  // assert(yaw == 0)
  // assert(pitch == 0)
  // assert(roll == 0)
  transform_type["get_yaw_pitch_roll"] = [](const Transform* transform) {
    std::tuple<float, float, float> yaw_pitch_roll;
    transform->GetYawPitchRoll(&std::get<0>(yaw_pitch_roll), &std::get<1>(yaw_pitch_roll),
                               &std::get<2>(yaw_pitch_roll));
    return yaw_pitch_roll;
  };

  /// Transforms a direcion from object to world space.
  // If the transform contains any scaling the magnitude of the vector will likely change.
  // @function object_space_direction_to_world_space
  // @param[type=Vector3] direction
  // @treturn Vector3
  // @see 03-spaces.md
  // @see world_direction_to_object_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:rotate(Vector3.POSITIVE_Y, 90)
  // local transformed_direction = transform:object_space_direction_to_world_space(Vector3.POSITIVE_X)
  // assert(Vector3.length(transformed_direction - Vector3.POSITIVE_Z) < 0.1)
  transform_type["object_space_direction_to_world_space"] = &Transform::ObjectSpaceDirectionToWorldSpace;

  /// Transforms a direcion from world to object space.
  // If the transform contains any scaling the magnitude of the vector will likely change.
  // @function world_space_direction_to_object_space
  // @param[type=Vector3] direction
  // @treturn Vector3
  // @see 03-spaces.md
  // @see object_direction_to_world_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:rotate(Vector3.POSITIVE_Y, 90)
  // local transformed_direction = transform:world_space_direction_to_object_space(Vector3.POSITIVE_Z)
  // assert(Vector3.length(transformed_direction - Vector3.POSITIVE_X) < 0.1)
  transform_type["world_space_direction_to_object_space"] = &Transform::WorldSpaceDirectionToObjectSpace;

  /// Transforms a position in local space to world space.
  // @function object_space_position_to_world_space
  // @param[type=Vector3] object_space_coordinates
  // @treturn Vector3
  // @see 03-spaces.md
  // @see world_position_to_local_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:move(Vector3.new(0, 1, 0))
  // local transformed_position = transform:object_space_position_to_world_space(Vector3.new(1, 0, 0))
  // assert(transformed_position == Vector3.new(1, 1, 0))
  transform_type["object_space_position_to_world_space"] = &Transform::ObjectSpacePositionToWorldSpace;

  /// Transforms a position in  world space to local space.
  // @function world_space_position_to_object_space
  // @param[type=Vector3] world_space_coordinates
  // @treturn Vector3
  // @see 03-spaces.md
  // @see local_position_to_world_space
  // @usage Vector3 = core.Vector3
  // local transform = Transform.new()
  // transform:move(Vector3.new(0, 1, 0))
  // local transformed_position = transform:world_space_position_to_object_space(Vector3.new(1, 1, 0))
  // assert(transformed_position == Vector3.new(1, 0, 0))
  transform_type["world_space_position_to_object_space"] = &Transform::WorldSpacePositionToObjectSpace;
}

void Transform::CalculateMatrices() const {
  local_to_world_ = Matrix3x4::FromTransformation(position_, scale_, rotation_);
  world_to_local_ = InvertAffine(local_to_world_);
  dirty = false;
}

}  // namespace ovis