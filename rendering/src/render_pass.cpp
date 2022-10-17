#include <ovis/rendering/render_pass.hpp>

namespace ovis {

RenderPass::RenderPass()
#if OVIS_ENABLE_BUILT_IN_PROFILING
    cpu_render_profiler_(fmt::format("{}::Render", name))
#endif
{
}

void RenderPass::RenderBefore(TypeId render_pass_type) {
  render_before_list_.insert(render_pass_type);
}

void RenderPass::RenderAfter(TypeId render_pass_type) {
  render_after_list_.insert(render_pass_type);
}

}  // namespace ovis
