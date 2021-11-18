#pragma once

#include <ovis/graphics/render_target_texture2d.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {

struct RenderTargetViewportDescription {
  RenderTargetTexture2DDescription color_description;
  std::optional<RenderTargetTexture2DDescription> depth_description;
};

class RenderTargetViewport : public RenderingViewport {
 public:
  RenderTargetViewport(GraphicsContext* graphics_context, const RenderTargetViewportDescription& description);

  void Resize(std::size_t width, std::size_t height);
  inline Vector2 GetDimensions() const override {
    return {static_cast<float>(description_.color_description.texture_description.width),
            static_cast<float>(description_.color_description.texture_description.height)};
  }
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
