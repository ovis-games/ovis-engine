#pragma once

#include <memory>
#include <type_traits>

#include <sol/sol.hpp>

#include <ovis/core/scene_viewport.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/graphics/render_target_texture2d.hpp>
#include <ovis/rendering/render_pass.hpp>
#include <ovis/vm/virtual_machine.hpp>

namespace ovis {

class RenderingViewport : public SceneViewport {
 public:
  RenderingViewport() = default;
  virtual ~RenderingViewport() = default;

  virtual RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() = 0;

  inline GraphicsContext* context() const { return graphics_context_; }

  template <typename RenderPassType> requires std::is_base_of_v<RenderPass, RenderPassType>
  Result<RenderPassType*> AddRenderPass();
  Result<RenderPass*> AddRenderPass(TypeId render_pass_type);
  void RemoveRenderPass(TypeId render_pass_type);
  template <typename RenderPassType = RenderPass> requires std::is_base_of_v<RenderPass, RenderPassType>
  inline const RenderPassType* GetRenderPass(TypeId render_pass_type) const {
    for (auto& render_pass : render_passes_) {
      if (render_pass.type_id() == render_pass_type) {
        return &render_pass.as<RenderPassType>();
      }
    }
    return nullptr;
  }
  template <typename RenderPassType = RenderPass> requires std::is_base_of_v<RenderPass, RenderPassType>
  inline RenderPassType* GetRenderPass(TypeId render_pass_type) {
    for (auto& render_pass : render_passes_) {
      if (render_pass.type_id() == render_pass_type) {
        return &render_pass.as<RenderPassType>();
      }
    }
    return nullptr;
  }
  inline void AddRenderPassDependency(TypeId rendered_first, TypeId rendered_second) {
    render_pass_dependencies_.insert(std::make_pair(std::move(rendered_second), std::move(rendered_first)));
  }

  RenderTargetTexture2D* CreateRenderTarget2D(const std::string& id,
                                              const RenderTargetTexture2DDescription& description);
  RenderTarget* GetRenderTarget(const std::string& id);

  std::unique_ptr<RenderTargetConfiguration> CreateRenderTargetConfiguration(
      std::vector<std::string> color_render_target_ids, std::string depth_render_target_id = "");

  virtual void Render();

 protected:
  void SetGraphicsContext(GraphicsContext* graphics_context);
  void ClearResources();

 private:
  void SortRenderPasses();

  GraphicsContext* graphics_context_ = nullptr;

  std::multimap<TypeId, TypeId> render_pass_dependencies_;
  std::vector<Value> render_passes_;
  std::vector<RenderPass*> render_pass_order_;
  bool render_passes_sorted_;
  std::unordered_map<std::string, std::unique_ptr<RenderTarget>> render_targets_;
};

template <typename RenderPassType> requires std::is_base_of_v<RenderPass, RenderPassType>
Result<RenderPassType*> RenderingViewport::AddRenderPass() {
  auto render_pass = AddRenderPass(main_vm->GetTypeId<RenderPassType>());
  OVIS_CHECK_RESULT(render_pass);
  return down_cast<RenderPassType*>(*render_pass);
}


}  // namespace ovis
