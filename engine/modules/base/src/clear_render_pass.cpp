#include <ovis/base/clear_render_pass.hpp>

#include <ovis/core/log.hpp>
#include <ovis/engine/viewport.hpp>

namespace ovis {

ClearRenderPass::ClearRenderPass() : RenderPass("Clear") {}

void ClearRenderPass::Render(const RenderContext& render_context) {
  viewport()->GetDefaultRenderTargetConfiguration()->ClearColor(0);
  viewport()->GetDefaultRenderTargetConfiguration()->ClearDepth();
}

}  // namespace ovis
