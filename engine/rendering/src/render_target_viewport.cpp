#include <ovis/rendering/render_target_viewport.hpp>

namespace ovis {

RenderTargetViewport::RenderTargetViewport(GraphicsContext* graphics_context, ResourceManager* resource_manager,
                                           const RenderTargetViewportDescription& description)
    : description_(description) {
  SetResourceManager(resource_manager);
  SetGraphicsContext(graphics_context);
  CreateRenderTargets();
}

void RenderTargetViewport::Resize(std::size_t width, std::size_t height) {
  description_.color_description.texture_description.width = width;
  description_.color_description.texture_description.height = height;
  if (description_.depth_description) {
    description_.color_description.texture_description.width = width;
    description_.color_description.texture_description.height = height;
  }
  CreateRenderTargets();
}

void RenderTargetViewport::GetDimensions(size_t* width, size_t* height) {
  *width = description_.color_description.texture_description.width;
  *height = description_.color_description.texture_description.height;
}

RenderTargetConfiguration* RenderTargetViewport::GetDefaultRenderTargetConfiguration() {
  return render_target_configuration_.get();
}

void RenderTargetViewport::CreateRenderTargets() {
  RenderTargetConfigurationDescription rtt_desc;
  color_ = std::make_unique<RenderTargetTexture2D>(context(), description_.color_description);
  rtt_desc.color_attachments = {color_.get()};

  if (description_.depth_description) {
    SDL_assert(description_.depth_description->texture_description.width =
                   description_.color_description.texture_description.width);
    SDL_assert(description_.depth_description->texture_description.height =
                   description_.color_description.texture_description.height);
    depth_ = std::make_unique<RenderTargetTexture2D>(context(), *description_.depth_description);
    rtt_desc.depth_attachment = depth_.get();
  }

  render_target_configuration_ = std::make_unique<RenderTargetConfiguration>(context(), rtt_desc);
}

}  // namespace ovis
