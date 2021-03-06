#include <SDL_assert.h>

#include <ovis/rendering2d/sprite_component.hpp>

namespace ovis {

const json SpriteComponent::schema = {{"$ref", "rendering2d#/$defs/sprite"}};

json SpriteComponent::Serialize() const {
  return {{"Size", size_},
          {"Color", color_},
          {"Texture", texture_asset_}};
}

bool SpriteComponent::Deserialize(const json& data) {
  try {
    if (data.contains("Size")) {
      size_ = data.at("Size");
    }
    if (data.contains("Color")) {
      color_ = data.at("Color");
    }
    if (data.contains("Texture")) {
      texture_asset_ = data.at("Texture");
    }
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace ovis