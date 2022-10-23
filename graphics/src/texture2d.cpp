#include "ovis/graphics/texture2d.hpp"

#include "ovis/utils/file.hpp"
#include "ovis/utils/log.hpp"
#include "ovis/graphics/graphics_context.hpp"

namespace ovis {

Texture2D::Texture2D(GraphicsContext* context, const Texture2DDescription& description, const void* pixels)
    : Texture(context, Type::TEXTURE_2D), m_description(description) {
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
      assert(pixels == nullptr);
      internal_format = GL_DEPTH_COMPONENT16;
      source_format = GL_DEPTH_COMPONENT;
      source_type = GL_FLOAT;
      break;

    case TextureFormat::DEPTH_UINT24:
      assert(pixels == nullptr);
      internal_format = GL_DEPTH_COMPONENT24;
      source_format = GL_DEPTH_COMPONENT;
      source_type = GL_FLOAT;
      break;

    case TextureFormat::DEPTH_FLOAT32:
      assert(pixels == nullptr);
      internal_format = GL_DEPTH_COMPONENT32F;
      source_format = GL_DEPTH_COMPONENT;
      source_type = GL_FLOAT;
      break;
#endif

    default:
      assert(false && "Invalid texture format");
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
      // assert(description.mip_map_count > 1);
      min_filter = GL_LINEAR_MIPMAP_LINEAR;
      mag_filter = GL_LINEAR;
      break;

    default:
      assert(false);
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
      assert(false);
      break;
  }

  glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLsizei>(level), static_cast<GLsizei>(x), static_cast<GLsizei>(y),
                  static_cast<GLsizei>(width), static_cast<GLsizei>(height), source_format, source_type, data);
}

void Texture2D::Bind(int texture_unit) {
  context()->BindTexture(GL_TEXTURE_2D, name(), texture_unit);
}

Result<Texture2DDescription> LoadTexture2DDescription(const std::string& asset_id) {
  return LoadTexture2DDescription(GetAssetLibraryForAsset(asset_id), asset_id);
}

Result<Texture2DDescription> LoadTexture2DDescription(AssetLibrary* asset_library, const std::string& asset_id) {
  const Result<std::string> parameters_file = asset_library->LoadAssetTextFile(asset_id, "json");
  OVIS_CHECK_RESULT(parameters_file);

  try {
    const json parameters = json::parse(*parameters_file);

    Texture2DDescription description;
    description.width = parameters["width"];
    description.height = parameters["height"];
    description.mip_map_count = 0;

    std::string filter = parameters["filter"];
    if (filter == "point") {
      description.filter = TextureFilter::POINT;
    } else if (filter == "bilinear") {
      description.filter = TextureFilter::BILINEAR;
    } else if (filter == "trilinear") {
      description.filter = TextureFilter::TRILINEAR;
    } else {
      return Error("Failed to load texture '{}': invalid filter ()", asset_id, filter);
    }

    std::string format = parameters["format"];
    if (format == "RGB_UINT8") {
      description.format = TextureFormat::RGB_UINT8;
    } else if (format == "RGBA_UINT8") {
      description.format = TextureFormat::RGBA_UINT8;
    } else {
      return Error("Failed to load texture '{}': invalid format ()", asset_id, format);
    }

    return description;
  } catch (const json::parse_error& error) {
    return Error("Invalid json: {}", error.what());
  }
}

std::unique_ptr<Texture2D> LoadTexture2D(const std::string& asset_id, GraphicsContext* graphics_context) {
  return LoadTexture2D(GetAssetLibraryForAsset(asset_id), asset_id, graphics_context);
}

std::unique_ptr<Texture2D> LoadTexture2D(AssetLibrary* asset_library, const std::string& asset_id,
                                         GraphicsContext* graphics_context) {
  assert(graphics_context != nullptr);

  Result<Texture2DDescription> description = LoadTexture2DDescription(asset_library, asset_id);
  Result<Blob> mip_level_data = asset_library->LoadAssetBinaryFile(asset_id, "0");

  if (description.has_value() && mip_level_data.has_value()) {
    return std::make_unique<Texture2D>(graphics_context, *description, mip_level_data->data());
  } else {
    LogE("Failed to load texture '{}'", asset_id);
    return {};
  }
}

}  // namespace ovis
