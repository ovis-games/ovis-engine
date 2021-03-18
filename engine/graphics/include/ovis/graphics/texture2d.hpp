#pragma once

#include <cstdlib>
#include <memory>

#include <ovis/graphics/texture.hpp>

namespace ovis {

class ResourceManager;

struct Texture2DDescription {
  std::size_t width;
  std::size_t height;
  std::size_t mip_map_count;
  TextureFormat format;
  TextureFilter filter;
};

class Texture2D : public Texture {
  friend class RenderTargetTexture2D;

 public:
  Texture2D(GraphicsContext* context, const Texture2DDescription& description, const void* pixels = nullptr);

  void GenerateMipMaps();

  void Write(std::size_t level, std::size_t x, std::size_t y, std::size_t width, std::size_t height, const void* data);

  inline const Texture2DDescription& description() const { return m_description; }

 private:
  Texture2DDescription m_description;

  virtual void Bind(int texture_unit) override;
};

// std::optional<Texture2DDescription> LoadTexture2DDescription(const std::string& asset_id);
// std::optional<Texture2DDescription> LoadTexture2DDescription(AssetLibrary* asset_library, const std::string& asset_id);

// std::unique_ptr<Texture2D> LoadTexture2D(const std::string& asset_id, GraphicsContext* graphics_context);
// std::unique_ptr<Texture2D> LoadTexture2D(AssetLibrary* asset_library, const std::string& asset_id,
//                                          GraphicsContext* graphics_context);

}  // namespace ovis
