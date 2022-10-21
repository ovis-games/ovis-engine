#include "ovis/rendering/clear_pass.hpp"

#include "ovis/graphics/render_target_configuration.hpp"

namespace ovis {

ClearPass::ClearPass(GraphicsContext* context, std::optional<Color> clear_color)
    : RenderPass("ClearPass", context), clear_color_(clear_color) {}

void ClearPass::Render(const SceneViewport&) {
  if (clear_color()) {
    context()->default_render_target_configuration()->ClearColor(0, clear_color()->data);
  }
  // if (clear_color_) {
  //   viewport()->GetDefaultRenderTargetConfiguration()->ClearColor(0, clear_color_->data);
  // }
  // if (clear_depth_) {
  //   viewport()->GetDefaultRenderTargetConfiguration()->ClearDepth(*clear_depth_);
  // }
}

}  // namespace ovis
