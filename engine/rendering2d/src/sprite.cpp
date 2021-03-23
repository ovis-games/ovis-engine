#include <SDL_assert.h>

#include <ovis/rendering2d/sprite.hpp>

namespace ovis {

const json Sprite::schema = {{"$ref", "rendering2d#/$defs/sprite"}};

json Sprite::Serialize() const {
  return {{"Size", size_}, {"Color", color_}, {"Texture", texture_asset_}};
}

bool Sprite::Deserialize(const json& data) {
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

void Sprite::RegisterType(sol::table* module) {
  /// A 2D graphic.
  // @classmod ovis.rendering2d.sprite
  sol::usertype<Sprite> sprite_type = module->new_usertype<Sprite>("Sprite", sol::no_constructor);

  /// The texture that is displayed.
  // @field[type=string] texture The asset name of the texture
  sprite_type["texture"] = sol::property(&Sprite::texture_asset, &Sprite::SetTexture);

  /// The size of the sprite.
  // @field[type=ovis.core.Vector2] size
  sprite_type["size"] = sol::property(&Sprite::size, &Sprite::SetSize);

  /// The color of the sprite.
  // If the sprite does not have a texture assigned it will be displayed as a solid color. Otherwise, the
  // color is used to tint the sprite by multiplying the sprite color with the individual colors of the
  // texture pixels. In that case setting the color to white (default) will leave the color of the texture
  // pixels as they are.
  // @field[type=ovis.core.Color] color
  sprite_type["color"] = sol::property(&Sprite::color, &Sprite::SetColor);
}

}  // namespace ovis