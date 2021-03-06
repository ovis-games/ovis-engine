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