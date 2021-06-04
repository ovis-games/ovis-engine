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
  explicit inline Transform(SceneObject* object) : SceneObjectComponent(object) {}

  inline Vector3 local_position() const { return position_; }
  inline void SetLocalPosition(Vector3 new_position) {
    position_ = new_position;
    FlagAsDirty();
  }
  inline void MoveLocally(Vector3 offset) {
    position_ += offset;
    FlagAsDirty();
  }

  inline Vector3 world_position() const { return ExtractColumn(local_to_world_matrix(), 3); }
  inline void SetWorldPosition(const Vector3 position) {
    SetLocalPosition(TransformPosition(world_to_local_matrix(), position));
  }
  inline void Move(Vector3 offset) {
    MoveLocally(TransformDirection(world_to_local_matrix(), offset));
  }

  inline Vector3 local_scale() const { return scale_; }
  inline void SetLocalScale(Vector3 new_scale) {
    scale_ = new_scale;
    FlagAsDirty();
  }
  inline void SetLocalScale(float new_uniform_scale) {
    scale_ = {new_uniform_scale, new_uniform_scale, new_uniform_scale};
    FlagAsDirty();
  }
  inline void ScaleLocally(Vector3 scale) {
    scale_ *= scale;
    FlagAsDirty();
  }
  inline void ScaleLocally(float uniform_scale) {
    scale_ *= uniform_scale;
    FlagAsDirty();
  }

  inline Quaternion local_rotation() const { return rotation_; }
  inline void SetLocalRotation(Quaternion new_rotation) {
    rotation_ = new_rotation;
    FlagAsDirty();
  }
  inline void RotateLocally(Quaternion rotation_offset) {
    rotation_ = rotation_offset * rotation_;
    FlagAsDirty();
  }
  inline void RotateLocally(Vector3 axis, float angle_in_radians) {
    RotateLocally(Quaternion::FromAxisAndAngle(axis, angle_in_radians));
    FlagAsDirty();
  }
  inline void SetLocalYawPitchRoll(float yaw, float pitch, float roll) {
    rotation_ = Quaternion::FromEulerAngles(yaw, pitch, roll);
    FlagAsDirty();
  }
  inline void GetLocalYawPitchRoll(float* yaw, float* pitch, float* roll) const {
    if (yaw) *yaw = ExtractYaw(rotation_);
    if (pitch) *pitch = ExtractPitch(rotation_);
    if (roll) *roll = ExtractRoll(rotation_);
  }

  inline Vector3 LocalDirectionToWorld(Vector3 local_space_direction) const {
    return TransformDirection(local_to_world_matrix(), local_space_direction);
  }

  inline Vector3 WorldDirectionToLocal(Vector3 world_space_direction) const {
    return TransformDirection(world_to_local_matrix(), world_space_direction);
  }

  inline Vector3 LocalPositionToWorld(Vector3 local_space_position) const {
    return TransformPosition(local_to_world_matrix(), local_space_position);
  }

  inline Vector3 WorldPositionToLocal(Vector3 world_space_position) const {
    return TransformPosition(world_to_local_matrix(), world_space_position);
  }

  inline Matrix3x4 local_to_world_matrix() const {
    if (dirty_) {
      CalculateMatrices();
    }
    return local_to_world_;
  }

  Matrix3x4 world_to_local_matrix() const {
    if (dirty_) {
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
  mutable bool dirty_ = true;

  // This method is const as the matrices are calculated "on-demand" in the getter functions
  void CalculateMatrices() const;

  // Set the dirty flag to the transform component and all tansform components of its children
  void FlagAsDirty();
  static void FlagAsDirty(SceneObject* object);
};

void to_json(json& data, const Transform& transform);
void from_json(const json& data, Transform& transform);

}  // namespace ovis
