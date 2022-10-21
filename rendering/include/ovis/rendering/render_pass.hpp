#pragma once

#include "ovis/core/scene_viewport.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/core/scene.hpp"

namespace ovis {

class RenderPass : public FrameJob {
  friend class RenderingViewport;

 public:
  RenderPass(std::string_view job_id, GraphicsContext* graphics_context);
  virtual ~RenderPass();

  Result<> Prepare(Scene* const& scene) override;
  Result<> Execute(const SceneUpdate& parameters) override;

  // inline SceneViewport* viewport() const { return viewport_; }
  inline GraphicsContext* context() const { return graphics_context_; }

  virtual void CreateResources() {}
  virtual void ReleaseResources() {}
  virtual void Render(const SceneViewport& viewport) = 0;

 private:
  GraphicsContext* graphics_context_ = nullptr;

#if OVIS_ENABLE_BUILT_IN_PROFILING
  std::unique_ptr<GPUTimeProfiler> gpu_render_profiler_;
#endif
};

}  // namespace ovis
