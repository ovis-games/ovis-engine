#include <ovis/base/clear_render_pass.hpp>

#include <ovis/core/log.hpp>
#include <ovis/engine/viewport.hpp>

namespace ovis {

ClearRenderPass::ClearRenderPass() : ovis::RenderPass("Clear") {}

void ClearRenderPass::Render(ovis::Scene* scene) {
  viewport()->GetDefaultRenderTargetConfiguration()->ClearColor(0);
  viewport()->GetDefaultRenderTargetConfiguration()->ClearDepth();
}

}  // namespace ovis
