#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <ovis/core/log.hpp>
#include <ovis/math/transform.hpp>

namespace ovis {

matrix4 Transform::CalculateMatrix() const {
  const auto translation_matrix = glm::translate(translation_);
  const auto rotation_matrix = glm::mat4_cast(rotation_);
  const auto scale_matrix = glm::scale(scale_);
  return translation_matrix * rotation_matrix * scale_matrix;
}

matrix4 Transform::CalculateInverseMatrix() const {
  const auto translation_matrix = glm::translate(-translation_);
  const auto rotation_matrix = glm::transpose(glm::mat4_cast(rotation_));
  const auto scale_matrix = glm::scale(1.0f / scale_);
  return scale_matrix * rotation_matrix * translation_matrix;
}

void to_json(json& data, const Transform& transform) {
  data["Position"] = transform.translation();
  data["Rotation"] = glm::eulerAngles(transform.rotaton()) * glm::one_over_pi<float>() * 180.0f;
  data["Scale"] = transform.scale();
}

void from_json(const json& data, Transform& transform) {
  if (data.contains("Position")) {
    const vector3 position = data.at("Position");
    transform.SetTranslation(position);
  }
  if (data.contains("Rotation")) {
    const vector3 euler_angles = vector3(data.at("Rotation")) * glm::pi<float>() / 180.0f;
    transform.SetRotation(glm::eulerAngleXYZ(euler_angles.x, euler_angles.y, euler_angles.z));
  }
  if (data.contains("Scale")) {
    const vector3 scale = data.at("Scale");
    transform.SetScale(scale);
  }
}

}  // namespace ovis