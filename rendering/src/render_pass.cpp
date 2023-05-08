#include <ovis/rendering/render_pass.hpp>

namespace ovis {

RenderPass::RenderPass(std::string_view job_id, GraphicsContext* graphics_context)
    : FrameJob(job_id), graphics_context_(graphics_context) {}

RenderPass::~RenderPass() {
  ReleaseResources();
}

Result<> RenderPass::Prepare(Scene* const& scene) {
  CreateResources();
  return Success;
}

Result<> RenderPass::Execute(const SceneUpdate& parameters) {
  SceneViewport viewport;
  Render(parameters, viewport);
  return Success;
}

}  // namespace ovis
