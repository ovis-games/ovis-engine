#pragma once

#include <ovis/graphics/render_target_texture2d.hpp>
#include <ovis/rendering/viewport.hpp>

namespace ovis {

struct RenderTargetViewportDescription {
  RenderTargetTexture2DDescription color_description;
  std::optional<RenderTargetTexture2DDescription> depth_description;
};

class RenderTargetViewport : public Viewport {
 public:
  RenderTargetViewport(GraphicsContext* graphics_context, ResourceManager* resource_manager,
                       const RenderTargetViewportDescription& description);

  void Resize(std::size_t width, std::size_t height);
  void GetDimensions(size_t* width, size_t* height) override;
  RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() override;
  inline RenderTargetTexture2D* color_texture() { return color_.get(); }
  inline RenderTargetTexture2D* depth_texture() { return depth_.get(); }

 private:
  RenderTargetViewportDescription description_;
  std::unique_ptr<RenderTargetTexture2D> color_;
  std::unique_ptr<RenderTargetTexture2D> depth_;
  std::unique_ptr<RenderTargetConfiguration> render_target_configuration_;

  void CreateRenderTargets();
};

}  // namespace ovis
