#include <ovis/graphics/texture.hpp>

namespace ovis {

Texture::Texture(GraphicsContext* context, Type type) : GraphicsResource(context, type) {
  glGenTextures(1, &name_);
}

Texture::~Texture() {
  glDeleteTextures(1, &name_);
}

}  // namespace ovis