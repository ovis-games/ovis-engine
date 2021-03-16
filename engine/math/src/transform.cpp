#include <tuple>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <ovis/core/log.hpp>
#include <ovis/math/transform.hpp>

namespace ovis {

Matrix4 Transform::CalculateMatrix() const {
  const auto translation_matrix = Matrix4::FromTranslation(position_);
  const auto rotation_matrix = Matrix4::FromRotation(rotation_);
  const auto scale_matrix = Matrix4::FromScaling(scale_);
  return translation_matrix * rotation_matrix * scale_matrix;
}

Matrix4 Transform::CalculateInverseMatrix() const {
  const auto translation_matrix = Matrix4::FromTranslation(-position_);
  const auto rotation_matrix = Transpose(Matrix4::FromRotation(rotation_));
  const auto scale_matrix = Matrix4::FromScaling(1.0f / scale_);
  return scale_matrix * rotation_matrix * translation_matrix;
}

void Transform::RegisterType(sol::table* module) {
  /// A class that respresents a 3D transformation of an object.
  // @classmod ovis.math.Transform
  // @usage local math = require "ovis.math"
  // local Transform = math.Transform
  sol::usertype<Transform> transform_type =
      module->new_usertype<Transform>("Transform", sol::constructors<Transform()>());

  /// The position of the transformation.
  // @field [type=Vector3] position
  transform_type["position"] = sol::property(&Transform::position, &Transform::SetPosition);

  /// The scale of the transformation.
  // @field [type=Vector3] scale
  transform_type["scale"] = sol::property(&Transform::scale, sol::resolve<void(Vector3)>(&Transform::SetScale));

  /// The rotation of the transformation.
  // @field [type=Quaternion] rotation
  transform_type["rotation"] = sol::property(&Transform::rotaton, &Transform::SetRotation);

  /// Moves the transformation.
  // @function move
  // @param [type=Vector3] offset
  // @usage local transform = Transform.new()
  // assert(transform.position == math.Vector3.ZERO)
  // transform:move(math.Vector3.POSITIVE_X)
  // assert(transform.position == math.Vector3.POSITIVE_X)
  // transform:move(math.Vector3.POSITIVE_X)
  // assert(transform.position == 2 * math.Vector3.POSITIVE_X)
  transform_type["move"] = &Transform::Move;

  // inline void SetScale(float new_uniform_scale) { scale_ = {new_uniform_scale, new_uniform_scale, new_uniform_scale};
  // } inline void Scale(Vector3 scale) { scale_ *= scale; } inline void Scale(float uniform_scale) { scale_ *=
  // uniform_scale; }

  /// Rotates the object by multiplying its rotation by another quaternion.
  // @function rotate
  // @param [type=Quaternion] rotation_offset

  /// Rotates the object by rotating it around an axis.
  // The angle is given in degrees.
  // @function rotate
  // @param [type=Vector3] axis
  // @param [type=number] angle
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

  // Vector3 TransformDirection(Vector3 direction) const { return rotation_ * direction; }

  // Matrix4 CalculateMatrix() const;
  // Matrix4 CalculateInverseMatrix() const;
}

void to_json(json& data, const Transform& transform) {
  data["Position"] = transform.position();
  data["Rotation"] = ExtractEulerAngles(transform.rotaton()) * RadiansToDegreesFactor<float>();
  data["Scale"] = transform.scale();
}

void from_json(const json& data, Transform& transform) {
  if (data.contains("Position")) {
    const Vector3 position = data.at("Position");
    transform.SetPosition(position);
  }
  if (data.contains("Rotation")) {
    const Vector3 euler_angles = Vector3(data.at("Rotation")) * DegreesToRadiansFactor<float>();
    transform.SetRotation(Quaternion::FromEulerAngles(euler_angles.x, euler_angles.y, euler_angles.z));
  }
  if (data.contains("Scale")) {
    const Vector3 scale = data.at("Scale");
    transform.SetScale(scale);
  }
}

}  // namespace ovis