#pragma once

#include <memory>

#include <ovis/core/resource_manager.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/graphics/render_target_texture2d.hpp>
#include <ovis/engine/render_pass.hpp>
#include <ovis/engine/scene.hpp>

namespace ovis {

class Viewport {
 public:
  Viewport() = default;
  virtual ~Viewport() = default;

  virtual glm::ivec2 GetSize() = 0;
  virtual RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() = 0;

  inline float GetAspectRatio() {
    const vector2 size = GetSize();
    return size.x / size.y;
  }

  inline GraphicsContext* context() const { return graphics_context_; }

  inline Scene* scene() const { return scene_; }
  inline void SetScene(Scene* scene) { scene_ = scene; }
  inline void SetCamera(const Camera& camera, const matrix4& camera_transform) {
    render_context_.scene = scene_;
    render_context_.camera = camera;
    render_context_.inverse_view_matrix = camera_transform;
    render_context_.view_matrix = glm::inverse(camera_transform);
    render_context_.projection_matrix = camera.CalculateProjectionMatrix();
    render_context_.inverse_projection_matrix = glm::inverse(render_context_.projection_matrix);
    render_context_.view_projection_matrix = render_context_.projection_matrix * render_context_.view_matrix;
  }

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

  RenderTargetTexture2D* CreateRenderTarget2D(const std::string& id,
                                              const RenderTargetTexture2DDescription& description);
  RenderTarget* GetRenderTarget(const std::string& id);

  std::unique_ptr<RenderTargetConfiguration> CreateRenderTargetConfiguration(
      std::vector<std::string> color_render_target_ids, std::string depth_render_target_id = "");

  virtual void DrawImGui() {}
  virtual void Render(bool render_gui = true);

  inline vector2 DeviceCoordinatesToNormalizedDeviceCoordinates(vector2 device_coordinates) {
    return (2.0f * device_coordinates / vector2(GetSize() - 1) - 1.0f) * vector2(1.0f, -1.0f);
  }

  inline vector2 NormalizedDeviceCoordinatesToDeviceCoordinates(vector2 normalized_device_coordinates) {
    return ((0.5f * vector2(1.0f, -1.0f) * normalized_device_coordinates) + 0.5f) * vector2(GetSize() - 1);
  }

  inline vector3 NormalizedDeviceCoordinatesToViewSpace(vector3 ndc) {
    return render_context_.inverse_projection_matrix * vector4(ndc, 1.0f);
  }

  inline vector3 DeviceCoordinatesToViewSpace(const vector2& device_coordinates, float normalized_depth = -1.0f) {
    return NormalizedDeviceCoordinatesToViewSpace(
        vector3(DeviceCoordinatesToNormalizedDeviceCoordinates(device_coordinates), normalized_depth));
  }

  inline vector3 DeviceCoordinatesToWorldSpace(const vector2& device_coordinates, float normalized_depth = -1.0f) {
    return render_context_.inverse_view_matrix *
           vector4(DeviceCoordinatesToViewSpace(device_coordinates, normalized_depth), 1.0f);
  }

 protected:
  void SetGraphicsContext(GraphicsContext* graphics_context);
  void SetResourceManager(ResourceManager* resource_manager);

 private:
  void SortRenderPasses();
  RenderPass* GetRenderPassInternal(const std::string& render_pass_name) const;

  GraphicsContext* graphics_context_ = nullptr;
  ResourceManager* resource_manager_ = nullptr;

  Scene* scene_ = nullptr;
  RenderContext render_context_;

  std::multimap<std::string, std::string> render_pass_dependencies_;
  std::unordered_map<std::string, std::unique_ptr<RenderPass>> render_passes_;
  std::vector<RenderPass*> render_pass_order_;
  bool render_passes_sorted_;
  std::unordered_map<std::string, std::unique_ptr<RenderTarget>> render_targets_;
};

}  // namespace ovis