#include <SDL_assert.h>

#include <ovis/rendering2d/sprite_component.hpp>
#include <ovis/math/vector.hpp>

namespace ovis {

// clang-format off
const json SpriteComponent::schema = {
  {"title", "Sprite"},
  {"type", "object"},
  {"properties", {
    {"Size", {
      {"type", "vector2"},
      {"description", "The size of the sprite."},
    }},
    {"Color", {
      {"type", "color"},
      {"description", "The color of the sprite."},
    }},
    {"Texture", {
      {"type", "asset<texture2d>"},
      {"description", "A texture that will be displayed on the sprite."},
    }}
  }}
};
// clang-format on

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