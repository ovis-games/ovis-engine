#pragma once

#include <ovis/math/basic_types.hpp>

namespace ovis {

class Transform {
 public:
  inline const vector3& translation() const { return translation_; }
  inline void SetTranslation(const vector3& new_translation) { translation_ = new_translation; }
  inline void Translate(const vector3& offset) { translation_ += offset; }

  inline vector3 scale() const { return scale_; }
  inline void SetScale(vector3 new_scale) { scale_ = new_scale; }
  inline void SetScale(float new_uniform_scale) { scale_ = {new_uniform_scale, new_uniform_scale, new_uniform_scale}; }
  inline void Scale(vector3 scale) { scale_ *= scale; }
  inline void Scale(float uniform_scale) { scale_ *= uniform_scale; }

  inline glm::quat rotaton() const { return rotation_; }
  inline void SetRotation(glm::quat new_rotation) { rotation_ = new_rotation; }
  inline void Rotate(glm::quat rotation_offset) { rotation_ = rotation_offset * rotation_; }
  inline void Rotate(vector3 axis, float angle_in_radians) { Rotate(glm::angleAxis(angle_in_radians, axis)); }
  inline void SetYawPitchRoll(float yaw, float pitch, float roll) {
    rotation_ = glm::angleAxis(yaw, vector3{0.0f, 1.0f, 0.0f}) * glm::angleAxis(pitch, vector3{-1.0f, 0.0f, 0.0f}) *
                glm::angleAxis(roll, vector3{0.0f, 0.0f, 1.0f});
  }
  inline void GetYawPitchRoll(float* yaw, float* pitch, float* roll) const {
    const vector3 euler = glm::eulerAngles(rotation_);
    *yaw = euler.y;
    *pitch = euler.x;
    *roll = euler.z;
  }

  vector3 TransformDirection(vector3 direction) const {
    return rotation_ * direction;
  }

  matrix4 CalculateMatrix() const;
  matrix4 CalculateInverseMatrix() const;

 private:
  vector3 translation_ = {0.0, 0.0, 0.0};
  vector3 scale_ = {1.0, 1.0, 1.0};
  glm::quat rotation_;
};

void to_json(json& data, const Transform& transform);
void from_json(const json& data, Transform& transform);

}  // namespace ovis
