#pragma once

#include <sol/sol.hpp>

#include <ovis/core/matrix.hpp>
#include <ovis/core/quaternion.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/vector.hpp>

namespace ovis {

class Transform : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();
  friend class TransformController;

 public:
  inline const Vector3& position() const { return position_; }
  inline void SetPosition(const Vector3& new_position) {
    position_ = new_position;
    dirty = true;
  }
  inline void Move(const Vector3& offset) {
    position_ += offset;
    dirty = true;
  }

  inline Vector3 scale() const { return scale_; }
  inline void SetScale(Vector3 new_scale) {
    scale_ = new_scale;
    dirty = true;
  }
  inline void SetScale(float new_uniform_scale) {
    scale_ = {new_uniform_scale, new_uniform_scale, new_uniform_scale};
    dirty = true;
  }
  inline void Scale(Vector3 scale) {
    scale_ *= scale;
    dirty = true;
  }
  inline void Scale(float uniform_scale) {
    scale_ *= uniform_scale;
    dirty = true;
  }

  inline Quaternion rotation() const { return rotation_; }
  inline void SetRotation(Quaternion new_rotation) {
    rotation_ = new_rotation;
    dirty = true;
  }
  inline void Rotate(Quaternion rotation_offset) {
    rotation_ = rotation_offset * rotation_;
    dirty = true;
  }
  inline void Rotate(Vector3 axis, float angle_in_radians) {
    Rotate(Quaternion::FromAxisAndAngle(axis, angle_in_radians));
    dirty = true;
  }
  inline void SetYawPitchRoll(float yaw, float pitch, float roll) {
    rotation_ = Quaternion::FromEulerAngles(yaw, pitch, roll);
    dirty = true;
  }
  inline void GetYawPitchRoll(float* yaw, float* pitch, float* roll) const {
    if (yaw) *yaw = ExtractYaw(rotation_);
    if (pitch) *pitch = ExtractPitch(rotation_);
    if (roll) *roll = ExtractRoll(rotation_);
  }

  inline Vector3 ObjectSpaceDirectionToWorldSpace(Vector3 object_space_direction) const {
    return TransformDirection(local_to_world_matrix(), object_space_direction);
  }

  inline Vector3 WorldSpaceDirectionToObjectSpace(Vector3 world_space_direction) const {
    return TransformDirection(world_to_local_matrix(), world_space_direction);
  }

  inline Vector3 ObjectSpacePositionToWorldSpace(Vector3 object_space_coordinates) const {
    return TransformPosition(local_to_world_matrix(), object_space_coordinates);
  }

  inline Vector3 WorldSpacePositionToObjectSpace(Vector3 world_space_coordinates) const {
    return TransformPosition(world_to_local_matrix(), world_space_coordinates);
  }

  inline Matrix3x4 local_to_world_matrix() const {
    if (dirty) {
      CalculateMatrices();
    }
    return local_to_world_;
  }

  Matrix3x4 world_to_local_matrix() const {
    if (dirty) {
      CalculateMatrices();
    }
    return world_to_local_;
  }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override;

  static void RegisterType(sol::table* module);

 private:
  Vector3 position_ = Vector3::Zero();
  Vector3 scale_ = Vector3::One();
  Quaternion rotation_ = Quaternion::Identity();

  mutable Matrix3x4 local_to_world_;
  mutable Matrix3x4 world_to_local_;

  // Indicates whether the transformation matrices need to be re-calculated
  mutable bool dirty = true;

  // This method is const as the matrices are calculated "on-demand" in the getter functions
  void CalculateMatrices() const;
};

void to_json(json& data, const Transform& transform);
void from_json(const json& data, Transform& transform);

}  // namespace ovis
