#include <SDL_assert.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <ovis/base/transform_component.hpp>

namespace ovis {

const json TransformComponent::schema = {{"$ref", "base#/$defs/transform"}};

json TransformComponent::Serialize() const {
  json data = json::object();
  data["Position"] = transform_.translation();
  data["Rotation"] = glm::eulerAngles(transform_.rotaton()) * glm::one_over_pi<float>() * 180.0f;
  data["Scale"] = transform_.scale();
  return data;
}

bool TransformComponent::Deserialize(const json& data) {
  try {
    if (data.contains("Position")) {
      const vector3 position = data.at("Position");
      transform_.SetTranslation(position);
    }
    if (data.contains("Rotation")) {
      const vector3 euler_angles = vector3(data.at("Rotation")) * glm::pi<float>() / 180.0f;
      transform_.SetRotation(glm::eulerAngleXYZ(euler_angles.x, euler_angles.y, euler_angles.z));
    }
    if (data.contains("Scale")) {
      const vector3 scale = data.at("Scale");
      transform_.SetScale(scale);
    }
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace ovis