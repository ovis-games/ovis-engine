#pragma once

#include <set>
#include <string>
#include <unordered_map>

#if OVIS_ENABLE_BUILT_IN_PROFILING
#include <ovis/utils/profiling.hpp>
#include <ovis/graphics/gpu_time_profiler.hpp>
#endif

#include <ovis/utils/static_factory.hpp>
#include <ovis/core/camera.hpp>
#include <ovis/core/matrix.hpp>
#include <ovis/core/scene.hpp>

namespace ovis {

class RenderingViewport;
class GraphicsContext;

struct RenderContext {
  Matrix3x4 world_to_view_space;
  Matrix3x4 view_to_world_space;
  Matrix4 view_to_clip_space;
  Matrix4 clip_to_view_space;
  Matrix4 world_to_clip_space;
};

class RenderPass : public StaticFactory<RenderPass, std::unique_ptr<RenderPass>()> {
  friend class RenderingViewport;

 public:
  RenderPass();
  virtual ~RenderPass() = default;

  inline RenderingViewport* viewport() const { return viewport_; }
  inline std::string name() const { return name_; }
  inline GraphicsContext* context() const { return graphics_context_; }

  virtual void CreateResources() {}
  virtual void ReleaseResources() {}
  virtual void Render(const RenderContext& render_context) = 0;

 protected:
  void RenderBefore(const std::string& renderer_reference);
  void RenderAfter(const std::string& renderer_reference);

 private:
  RenderingViewport* viewport_ = nullptr;
  GraphicsContext* graphics_context_ = nullptr;
  std::string name_;
  std::set<TypeId> render_before_list_;
  std::set<TypeId> render_after_list_;

#if OVIS_ENABLE_BUILT_IN_PROFILING
  CPUTimeProfiler cpu_render_profiler_;
  std::unique_ptr<GPUTimeProfiler> gpu_render_profiler_;
#endif

  inline void RenderWrapper(const RenderContext& render_context) {
#if OVIS_ENABLE_BUILT_IN_PROFILING
    cpu_render_profiler_.BeginMeasurement();
    gpu_render_profiler_->BeginMeasurement();
#endif
    Render(render_context);
#if OVIS_ENABLE_BUILT_IN_PROFILING
    cpu_render_profiler_.EndMeasurement();
    gpu_render_profiler_->EndMeasurement();
#endif
  }

  inline void CreateResourcesWrapper() {
#if OVIS_ENABLE_BUILT_IN_PROFILING
    gpu_render_profiler_ = std::make_unique<GPUTimeProfiler>(context(), name() + "::Render");
#endif
    CreateResources();
  }
  inline void ReleaseResourcesWrapper() {
#if OVIS_ENABLE_BUILT_IN_PROFILING
    gpu_render_profiler_.reset();
#endif
    ReleaseResources();
  }
};

}  // namespace ovis
