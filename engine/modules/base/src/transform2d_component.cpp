#include <SDL_assert.h>
#include <ovis/base/transform2d_component.hpp>

#include <ovis/math/json_serialization.hpp>

namespace ovis {

const json Transform2DComponent::schema = {{"$ref", "base#/$defs/transform2d"}};

json Transform2DComponent::Serialize() const {
  return {{"Position", glm::vec2(transform_.translation())},
          {"Rotation", transform_.rotaton().z},
          {"Scale", glm::vec2(transform_.scale())}};
}

bool Transform2DComponent::Deserialize(const json& data) {
  try {
    if (data.contains("Position")) {
      const glm::vec2 position = data.at("Position");
      transform_.SetTranslation(glm::vec3(position, 0.0f));
    }
    if (data.contains("Rotation")) {
      const float rotation = data.at("Rotation");
      transform_.SetRotation(glm::quat(1.0f, 0.0f, 0.0f, rotation));
    }
    if (data.contains("Scale")) {
      const glm::vec2 scaling = data.at("Scale");
      transform_.SetScale(glm::vec3(scaling, 1.0f));
    }
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace ovis