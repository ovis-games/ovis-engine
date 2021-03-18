#pragma once

#include <memory>

#include <sol/sol.hpp>

#include <ovis/core/resource_manager.hpp>
#include <ovis/scene/scene.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/graphics/render_target_texture2d.hpp>
#include <ovis/rendering/render_pass.hpp>

namespace ovis {

class Viewport {
 public:
  Viewport() = default;
  virtual ~Viewport() = default;

  virtual void GetDimensions(size_t* width, size_t* height) = 0;
  virtual RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() = 0;

  inline Vector2 GetDimensionsAsVector2() {
    size_t width;
    size_t height;
    GetDimensions(&width, &height);
    return {static_cast<float>(width), static_cast<float>(height)};
  }

  inline float GetAspectRatio() {
    const Vector2 size = GetDimensionsAsVector2();
    return size.x / size.y;
  }

  inline GraphicsContext* context() const { return graphics_context_; }

  inline Scene* scene() const { return scene_; }
  inline void SetScene(Scene* scene) { scene_ = scene; }
  inline void SetCamera(const Camera& camera, const Matrix4& camera_transform) {
    render_context_.scene = scene_;
    render_context_.camera = camera;
    render_context_.inverse_view_matrix = camera_transform;
    render_context_.view_matrix = Invert(camera_transform);  // TODO: use faster invert method
    render_context_.projection_matrix = camera.CalculateProjectionMatrix();
    render_context_.inverse_projection_matrix = Invert(render_context_.projection_matrix);
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

  inline Vector2 DeviceCoordinatesToNormalizedDeviceCoordinates(Vector2 device_coordinates) {
    return (2.0f * device_coordinates / (GetDimensionsAsVector2() - Vector2::One()) - Vector2::One()) *
           Vector2{1.0f, -1.0f};
  }

  inline Vector2 NormalizedDeviceCoordinatesToDeviceCoordinates(Vector2 normalized_device_coordinates) {
    return ((0.5f * Vector2{1.0f, -1.0f} * normalized_device_coordinates) + Vector2{0.5f, 0.5f}) *
           (GetDimensionsAsVector2() - Vector2::One());
  }

  inline Vector3 NormalizedDeviceCoordinatesToViewSpace(Vector3 ndc) {
    return TransformPosition(render_context_.inverse_projection_matrix, ndc);
  }

  inline Vector3 DeviceCoordinatesToViewSpace(const Vector2& device_coordinates, float normalized_depth = -1.0f) {
    const Vector2 ndc = DeviceCoordinatesToNormalizedDeviceCoordinates(device_coordinates);
    return NormalizedDeviceCoordinatesToViewSpace(Vector3::FromVector2(ndc, normalized_depth));
  }

  inline Vector3 DeviceCoordinatesToWorldSpace(const Vector2& device_coordinates, float normalized_depth = -1.0f) {
    return TransformPosition(render_context_.inverse_view_matrix,
                             DeviceCoordinatesToViewSpace(device_coordinates, normalized_depth));
  }

  // You should never call this manually. This function is only there to implement copying in the web browser
  void ComputeImGuiFrame();

  static void RegisterType(sol::table* module);

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