#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_surface.h>

#include <ovis/core/file.hpp>
#include <ovis/core/log.hpp>
#include <ovis/core/resource_manager.hpp>
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

bool LoadTexture2D(GraphicsContext* graphics_context, ResourceManager* resource_manager, const json& parameters,
                   const std::string& id, const std::string& directory) {
  Texture2DDescription texture2d_desc;
  texture2d_desc.width = parameters["width"];
  texture2d_desc.height = parameters["height"];
  texture2d_desc.mip_map_count = 0;

  std::string filter = parameters["filter"];
  if (filter == "point") {
    texture2d_desc.filter = TextureFilter::POINT;
  } else if (filter == "bilinear") {
    texture2d_desc.filter = TextureFilter::BILINEAR;
  } else if (filter == "trilinear") {
    texture2d_desc.filter = TextureFilter::TRILINEAR;
  } else {
    LogE("Failed to load texture '{}': invalid filter ()", id, filter);
    return false;
  }

  std::string format = parameters["format"];
  if (format == "RGB_UINT8") {
    texture2d_desc.format = TextureFormat::RGB_UINT8;
  } else if (format == "RGBA_UINT8") {
    texture2d_desc.format = TextureFormat::RGBA_UINT8;
  } else {
    LogE("Failed to load texture '{}': invalid format ()", id, format);
    return false;
  }

  auto file_content = LoadBinaryFile(directory + "/" + parameters["data_file"].get<std::string>());

  if (file_content.has_value()) {
    resource_manager->RegisterResource<Texture2D>(id, graphics_context, texture2d_desc, file_content->data());
    LogI("Sucessfully loaded texture: {}", id);
    return true;
  } else {
    LogE("Cannot open {}", parameters["data_file"]);
    return false;
  }
}

std::unique_ptr<Texture2D> LoadTexture2D(const std::string& asset_id, GraphicsContext* graphics_context) {
  return LoadTexture2D(
      GetApplicationAssetLibrary()->Contains(asset_id) ? GetApplicationAssetLibrary() : GetEngineAssetLibrary(),
      asset_id, graphics_context);
}

std::unique_ptr<Texture2D> LoadTexture2D(AssetLibrary* asset_library, const std::string& asset_id,
                                         GraphicsContext* graphics_context) {
  const std::optional<std::string> parameters_file = asset_library->LoadAssetTextFile(asset_id, "json");

  if (!parameters_file) {
    LogE("Cannot find parameters file for texture asset '{}'", asset_id);
    return {};
  }

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
      LogE("Failed to load texture '{}': invalid filter ()", asset_id, filter);
      return {};
    }

    std::string format = parameters["format"];
    if (format == "RGB_UINT8") {
      description.format = TextureFormat::RGB_UINT8;
    } else if (format == "RGBA_UINT8") {
      description.format = TextureFormat::RGBA_UINT8;
    } else {
      LogE("Failed to load texture '{}': invalid format ()", asset_id, format);
      return {};
    }

    std::optional<ovis::Blob> mip_level_data = asset_library->LoadAssetBinaryFile(asset_id, "0");

    if (mip_level_data.has_value()) {
      return std::make_unique<Texture2D>(graphics_context, description, mip_level_data->data());
    } else {
      LogE("Failed to load mip map level 0 for texture '{}'", asset_id);
      return {};
    }
  } catch (const ovis::json::parse_error& error) {
    ovis::LogE("Invalid json: {}", error.what());
    return {};
  }
}

}  // namespace ovis
