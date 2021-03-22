#include <ovis/rendering/graphics_loader.hpp>

namespace ovis {

std::unique_ptr<ShaderProgram> LoadShaderProgram(const std::string& asset_id, GraphicsContext* graphics_context) {
  return LoadShaderProgram(
      GetApplicationAssetLibrary()->Contains(asset_id) ? GetApplicationAssetLibrary() : GetEngineAssetLibrary(),
      asset_id, graphics_context);
}

std::unique_ptr<ShaderProgram> LoadShaderProgram(AssetLibrary* asset_library, const std::string& asset_id,
                                                 GraphicsContext* graphics_context) {
  ShaderProgramDescription description;

  if (!asset_library->Contains(asset_id)) {
    LogE("Cannot find asset '{}'", asset_id);
    return {};
  }

  std::optional<std::string> vertex_shader_source = asset_library->LoadAssetTextFile(asset_id, "vert");
  if (!vertex_shader_source) {
    LogE("Shader program '{}' does not have a corresponding vertex shader", asset_id);
    return {};
  } else {
    description.vertex_shader_source = *vertex_shader_source;
  }

  std::optional<std::string> fragment_shader_source = asset_library->LoadAssetTextFile(asset_id, "frag");
  if (!fragment_shader_source) {
    LogE("Shader program '{}' does not have a corresponding fragment shader", asset_id);
    return {};
  } else {
    description.fragment_shader_source = *fragment_shader_source;
  }

  return std::make_unique<ShaderProgram>(graphics_context, description);
}

std::optional<Texture2DDescription> LoadTexture2DDescription(const std::string& asset_id) {
  return LoadTexture2DDescription(GetAssetLibraryForAsset(asset_id), asset_id);
}

std::optional<Texture2DDescription> LoadTexture2DDescription(AssetLibrary* asset_library, const std::string& asset_id) {
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

    return description;
  } catch (const json::parse_error& error) {
    LogE("Invalid json: {}", error.what());
    return {};
  }
}

std::unique_ptr<Texture2D> LoadTexture2D(const std::string& asset_id, GraphicsContext* graphics_context) {
  return LoadTexture2D(GetAssetLibraryForAsset(asset_id), asset_id, graphics_context);
}

std::unique_ptr<Texture2D> LoadTexture2D(AssetLibrary* asset_library, const std::string& asset_id,
                                         GraphicsContext* graphics_context) {
  SDL_assert(graphics_context != nullptr);

  std::optional<Texture2DDescription> description = LoadTexture2DDescription(asset_library, asset_id);
  std::optional<Blob> mip_level_data = asset_library->LoadAssetBinaryFile(asset_id, "0");

  if (description.has_value() && mip_level_data.has_value()) {
    return std::make_unique<Texture2D>(graphics_context, *description, mip_level_data->data());
  } else {
    LogE("Failed to load texture '{}'", asset_id);
    return {};
  }
}

}  // namespace ovis
