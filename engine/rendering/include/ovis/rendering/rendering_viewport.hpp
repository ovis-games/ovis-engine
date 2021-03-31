#pragma once

#include <memory>

#include <sol/sol.hpp>

#include <ovis/core/scene_viewport.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/graphics/render_target_texture2d.hpp>
#include <ovis/rendering/render_pass.hpp>

namespace ovis {

class RenderingViewport : public SceneViewport {
 public:
  RenderingViewport() = default;
  virtual ~RenderingViewport() = default;

  virtual RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() = 0;

  inline GraphicsContext* context() const { return graphics_context_; }

  RenderPass* AddRenderPass(std::unique_ptr<RenderPass> render_pass);
  RenderPass* AddRenderPass(const std::string& render_pass_id);
  void RemoveRenderPass(const std::string& render_pass_id);
  template <typename RenderPassType = RenderPass>
  inline RenderPassType* GetRenderPass(const std::string& render_pass_name) const {
    static_assert(std::is_base_of<RenderPass, RenderPassType>::value, "");
    return down_cast<RenderPassType*>(GetRenderPassInternal(render_pass_name));
  }
  inline void AddRenderPassDependency(std::string rendered_first, std::string rendered_second) {
    render_pass_dependencies_.insert(std::make_pair(std::move(rendered_second), std::move(rendered_first)));
  }
  inline void AddRenderPassDependency(std::string_view rendered_first, std::string_view rendered_second) {
    render_pass_dependencies_.insert(std::make_pair(std::string(rendered_second), std::string(rendered_first)));
  }

  RenderTargetTexture2D* CreateRenderTarget2D(const std::string& id,
                                              const RenderTargetTexture2DDescription& description);
  RenderTarget* GetRenderTarget(const std::string& id);

  std::unique_ptr<RenderTargetConfiguration> CreateRenderTargetConfiguration(
      std::vector<std::string> color_render_target_ids, std::string depth_render_target_id = "");

  virtual void Render();

 protected:
  void SetGraphicsContext(GraphicsContext* graphics_context);

 private:
  void SortRenderPasses();
  RenderPass* GetRenderPassInternal(const std::string& render_pass_name) const;

  GraphicsContext* graphics_context_ = nullptr;

  std::multimap<std::string, std::string> render_pass_dependencies_;
  std::unordered_map<std::string, std::unique_ptr<RenderPass>> render_passes_;
  std::vector<RenderPass*> render_pass_order_;
  bool render_passes_sorted_;
  std::unordered_map<std::string, std::unique_ptr<RenderTarget>> render_targets_;
};

}  // namespace ovis