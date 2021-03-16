#pragma once

#include <sol/sol.hpp>

#include <ovis/math/matrix.hpp>
#include <ovis/math/quaternion.hpp>
#include <ovis/math/vector.hpp>

namespace ovis {

class Transform {
 public:
  inline const Vector3& position() const { return position_; }
  inline void SetPosition(const Vector3& new_position) { position_ = new_position; }
  inline void Move(const Vector3& offset) { position_ += offset; }

  inline Vector3 scale() const { return scale_; }
  inline void SetScale(Vector3 new_scale) { scale_ = new_scale; }
  inline void SetScale(float new_uniform_scale) { scale_ = {new_uniform_scale, new_uniform_scale, new_uniform_scale}; }
  inline void Scale(Vector3 scale) { scale_ *= scale; }
  inline void Scale(float uniform_scale) { scale_ *= uniform_scale; }

  inline Quaternion rotaton() const { return rotation_; }
  inline void SetRotation(Quaternion new_rotation) { rotation_ = new_rotation; }
  inline void Rotate(Quaternion rotation_offset) { rotation_ = rotation_offset * rotation_; }
  inline void Rotate(Vector3 axis, float angle_in_radians) {
    Rotate(Quaternion::FromAxisAndAngle(axis, angle_in_radians));
  }
  inline void SetYawPitchRoll(float yaw, float pitch, float roll) {
    rotation_ = Quaternion::FromEulerAngles(yaw, pitch, roll);
  }
  inline void GetYawPitchRoll(float* yaw, float* pitch, float* roll) const {
    if (yaw) *yaw = ExtractYaw(rotation_);
    if (pitch) *pitch = ExtractPitch(rotation_);
    if (roll) *roll = ExtractRoll(rotation_);
  }

  Vector3 TransformDirection(Vector3 direction) const { return rotation_ * direction; }

  Matrix4 CalculateMatrix() const;
  Matrix4 CalculateInverseMatrix() const;

  static void RegisterType(sol::table* module);

 private:
  Vector3 position_ = Vector3::Zero();
  Vector3 scale_ = Vector3::One();
  Quaternion rotation_ = Quaternion::Identity();
};

void to_json(json& data, const Transform& transform);
void from_json(const json& data, Transform& transform);

}  // namespace ovis
