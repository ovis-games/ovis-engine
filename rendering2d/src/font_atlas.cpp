#include <ovis/rendering2d/font_atlas.hpp>

namespace ovis {

FontAtlas::FontAtlas(GraphicsContext* context, std::string_view asset, float height) : asset_(asset), height_(height) {
  assert(context != nullptr);


  Texture2DDescription texture_description {
    .width = 512,
    .height = 512,
    .mip_map_count = 0,
    .format = TextureFormat::RGBA_UINT8,
    .filter = TextureFilter::TRILINEAR,
  };
  std::vector<unsigned char> pixels(texture_description.width * texture_description.height);

  const auto* asset_library = GetAssetLibraryForAsset(asset);
  if (!asset_library) {
    // TODO: throw some kind of error here
  }
  assert(asset_library != nullptr);
  const auto file = asset_library->LoadAssetBinaryFile(asset, "ttf");
  if (!file.has_value()) {
    // TODO: throw some kind of error here
  }
  assert(file.has_value());

  int result =
      stbtt_BakeFontBitmap(reinterpret_cast<const unsigned char*>(file->data()), 0, height, pixels.data(),
                           texture_description.width, texture_description.height, 32, 96, baked_characters.data());
  assert(result > 0);

  // TODO: this texture format stores 1 byte worth of data in 4 bytes. Use a 1 byte/pixel texture.
  std::vector<unsigned char> wasteful_pixels;
  wasteful_pixels.reserve(texture_description.width * texture_description.height * 4);
  int min = 255;
  for (auto p : pixels) {
    wasteful_pixels.push_back(255);
    wasteful_pixels.push_back(255);
    wasteful_pixels.push_back(255);
    wasteful_pixels.push_back(p);
    if (p < min) {
      min = p;
    }
  }
  texture_ = std::make_unique<Texture2D>(context, texture_description, wasteful_pixels.data());
}

std::array<Shape2D::Vertex, 6> FontAtlas::GetCharacterVertices(char character, Vector2* current_position, Color color) {

  stbtt_aligned_quad quad;
  stbtt_GetBakedQuad(baked_characters.data(), texture_->description().width, texture_->description().width,
                     character - 32, &current_position->x, &current_position->y, &quad, 1);

  uint32_t color_rgba = ConvertToRGBA8(color);

  return {{
    { .x = quad.x0, .y = -quad.y0, .s = quad.s0, .t = quad.t0, .color = color_rgba },
    { .x = quad.x1, .y = -quad.y0, .s = quad.s1, .t = quad.t0, .color = color_rgba },
    { .x = quad.x1, .y = -quad.y1, .s = quad.s1, .t = quad.t1, .color = color_rgba },
    { .x = quad.x0, .y = -quad.y0, .s = quad.s0, .t = quad.t0, .color = color_rgba },
    { .x = quad.x1, .y = -quad.y1, .s = quad.s1, .t = quad.t1, .color = color_rgba },
    { .x = quad.x0, .y = -quad.y1, .s = quad.s0, .t = quad.t1, .color = color_rgba },
  }};
}

}  // namespace ovis
