#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_surface.h>

#include <ovis/utils/file.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/texture2d.hpp>

namespace ovis {

Texture2D::Texture2D(GraphicsContext* context, const Texture2DDescription& description, const void* pixels)
    : Texture(context), m_description(description) {
  Bind(0);

  GLenum internal_format;
  GLenum source_format;
  GLenum source_type;
  switch (description.format) {
    case TextureFormat::RGB_UINT8:
      internal_format = GL_RGB;
      source_format = GL_RGB;
      source_type = GL_UNSIGNED_BYTE;
      break;

    case TextureFormat::RGBA_UINT8:
      internal_format = GL_RGBA;
      source_format = GL_RGBA;
      source_type = GL_UNSIGNED_BYTE;
      break;

#if !OVIS_EMSCRIPTEN
    case TextureFormat::RGBA_FLOAT32:
      internal_format = GL_RGBA32F;
      source_format = GL_RGBA;
      source_type = GL_FLOAT;
      break;

    case TextureFormat::DEPTH_UINT16:
      SDL_assert(pixels == nullptr);
      internal_format = GL_DEPTH_COMPONENT16;
      source_format = GL_DEPTH_COMPONENT;
      source_type = GL_FLOAT;
      break;

    case TextureFormat::DEPTH_UINT24:
      SDL_assert(pixels == nullptr);
      internal_format = GL_DEPTH_COMPONENT24;
      source_format = GL_DEPTH_COMPONENT;
      source_type = GL_FLOAT;
      break;

    case TextureFormat::DEPTH_FLOAT32:
      SDL_assert(pixels == nullptr);
      internal_format = GL_DEPTH_COMPONENT32F;
      source_format = GL_DEPTH_COMPONENT;
      source_type = GL_FLOAT;
      break;
#endif

    default:
      SDL_assert(false && "Invalid texture format");
      break;
  }

  if (pixels == nullptr) {
#if 0
    glTexStorage2D(GL_TEXTURE_2D, 0, internal_format,
                   static_cast<GLsizei>(description.width),
                   static_cast<GLsizei>(description.height));
#else
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, static_cast<GLsizei>(description.width),
                 static_cast<GLsizei>(description.height), 0, source_format, source_type, pixels);
#endif
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, static_cast<GLsizei>(description.width),
                 static_cast<GLsizei>(description.height), 0, source_format, source_type, pixels);
  }
  if (description.mip_map_count != 1) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  GLenum min_filter;
  GLenum mag_filter;
  switch (description.filter) {
    case TextureFilter::POINT:
      min_filter = description.mip_map_count > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      mag_filter = GL_NEAREST;
      break;

    case TextureFilter::BILINEAR:
      min_filter = description.mip_map_count > 1 ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
      mag_filter = GL_LINEAR;
      break;

    case TextureFilter::TRILINEAR:
      // SDL_assert(description.mip_map_count > 1);
      min_filter = GL_LINEAR_MIPMAP_LINEAR;
      mag_filter = GL_LINEAR;
      break;

    default:
      SDL_assert(false);
      break;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture2D::Write(std::size_t level, std::size_t x, std::size_t y, std::size_t width, std::size_t height,
                      const void* data) {
  Bind(0);

  GLenum source_format;
  GLenum source_type;
  switch (m_description.format) {
    case TextureFormat::RGB_UINT8:
      source_format = GL_RGB;
      source_type = GL_UNSIGNED_BYTE;
      break;

    case TextureFormat::RGBA_UINT8:
      source_format = GL_RGBA;
      source_type = GL_UNSIGNED_BYTE;
      break;

    default:
      SDL_assert(false);
      break;
  }

  glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLsizei>(level), static_cast<GLsizei>(x), static_cast<GLsizei>(y),
                  static_cast<GLsizei>(width), static_cast<GLsizei>(height), source_format, source_type, data);
}

void Texture2D::Bind(int texture_unit) {
  context()->BindTexture(GL_TEXTURE_2D, name(), texture_unit);
}

}  // namespace ovis
