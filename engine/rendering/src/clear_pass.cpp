#include <ovis/rendering/clear_pass.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {

ClearPass::ClearPass() : RenderPass("Clear") {}

void ClearPass::Render(const RenderContext&) {
  if (clear_color_) {
    viewport()->GetDefaultRenderTargetConfiguration()->ClearColor(0, clear_color_->data);
  }
  if (clear_depth_) {
    viewport()->GetDefaultRenderTargetConfiguration()->ClearDepth(*clear_depth_);
  }
}

}  // namespace ovis
