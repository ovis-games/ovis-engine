#pragma once

#include <memory>

#include <ovis/core/asset_library.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/texture2d.hpp>

namespace ovis {

std::unique_ptr<ShaderProgram> LoadShaderProgram(const std::string& asset_id, GraphicsContext* graphics_context);
std::unique_ptr<ShaderProgram> LoadShaderProgram(AssetLibrary* asset_library, const std::string& asset_id,
                                                 GraphicsContext* graphics_context);

std::optional<Texture2DDescription> LoadTexture2DDescription(const std::string& asset_id);
std::optional<Texture2DDescription> LoadTexture2DDescription(AssetLibrary* asset_library, const std::string& asset_id);

std::unique_ptr<Texture2D> LoadTexture2D(const std::string& asset_id, GraphicsContext* graphics_context);
std::unique_ptr<Texture2D> LoadTexture2D(AssetLibrary* asset_library, const std::string& asset_id,
                                         GraphicsContext* graphics_context);

}  // namespace ovis
