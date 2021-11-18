#include <ovis/rendering/render_pass.hpp>

namespace ovis {

RenderPass::RenderPass(std::string_view name)
    : name_(name)
#if OVIS_ENABLE_BUILT_IN_PROFILING
      ,
      cpu_render_profiler_(fmt::format("{}::Render", name))
#endif
{
}

void RenderPass::RenderBefore(const std::string& render_pass_name) {
  render_before_list_.insert(render_pass_name);
}

void RenderPass::RenderAfter(const std::string& render_pass_name) {
  render_after_list_.insert(render_pass_name);
}

}  // namespace ovis
