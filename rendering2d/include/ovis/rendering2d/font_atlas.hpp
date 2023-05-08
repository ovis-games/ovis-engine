#pragma once

#include <array>
#include <string_view>

#include "stb_truetype.h"

#include "ovis/graphics/texture2d.hpp"
#include "ovis/rendering2d/shape2d.hpp"

namespace ovis {

class FontAtlas {
  public:
    FontAtlas(GraphicsContext* context, std::string_view asset, float height);

    std::string_view asset() const { return asset_; }
    float height() const { return height_; }
    Texture2D* texture() { return texture_.get(); }

    std::array<Shape2D::Vertex, 6> GetCharacterVertices(char character, Vector2* current_position, Color color);

  private:
    std::string asset_;
    float height_;
    std::unique_ptr<Texture2D> texture_;
    std::array<stbtt_bakedchar, 96> baked_characters; // Storage for printable ASCII characters
};

}
