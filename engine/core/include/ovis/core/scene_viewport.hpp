#pragma once

#include <memory>

#include <sol/sol.hpp>

#include <ovis/core/camera.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/transform.hpp>

namespace ovis {

class SceneViewport {
 public:
  SceneViewport() = default;
  virtual ~SceneViewport() = default;

  // Float precisiton should be plenty, as single precision floats can represent integers between
  // 0 and 16777216 exactly.
  virtual Vector2 GetDimensions() = 0;

  inline float GetAspectRatio() {
    const Vector2 size = GetDimensions();
    return size.x / size.y;
  }

  inline Scene* scene() const { return scene_; }
  inline void SetScene(Scene* scene) { scene_ = scene; }

  inline Camera* camera() const { return camera_; }
  inline void SetCamera(Camera* camera) {
    SDL_assert(camera == nullptr || scene_ == camera->scene_object()->scene());
    camera_ = camera;
  }
  inline void SetCustomCameraMatrices(Matrix3x4 world_to_view_space, Matrix4 view_to_clip_space) {
    custom_world_to_view_space_ = world_to_view_space;
    custom_view_to_world_space_ = InvertAffine(world_to_view_space);
    custom_view_to_clip_space_ = view_to_clip_space;
    custom_clip_to_view_space_ = Invert(custom_view_to_clip_space_);
  }

  inline Vector2 DeviceCoordinatesToNormalizedDeviceCoordinates(Vector2 device_coordinates) {
    return (2.0f * device_coordinates / (GetDimensions() - Vector2::One()) - Vector2::One()) * Vector2{1.0f, -1.0f};
  }

  inline Vector2 NormalizedDeviceCoordinatesToDeviceCoordinates(Vector2 normalized_device_coordinates) {
    return ((0.5f * Vector2{1.0f, -1.0f} * normalized_device_coordinates) + Vector2{0.5f, 0.5f}) *
           (GetDimensions() - Vector2::One());
  }

  inline Vector3 NormalizedDeviceCoordinatesToViewSpace(Vector3 ndc) {
    return camera_ ? camera_->NormalizedDeviceCoordinatesToViewSpace(ndc)
                   : TransformPosition(custom_clip_to_view_space_, ndc);
  }

  inline Vector3 DeviceCoordinatesToViewSpace(const Vector2& device_coordinates, float normalized_depth = -1.0f) {
    const Vector2 ndc = DeviceCoordinatesToNormalizedDeviceCoordinates(device_coordinates);
    return NormalizedDeviceCoordinatesToViewSpace(Vector3::FromVector2(ndc, normalized_depth));
  }

  inline Vector3 DeviceCoordinatesToWorldSpace(const Vector2& device_coordinates, float normalized_depth = -1.0f) {
    const Vector3 view_space_position = DeviceCoordinatesToViewSpace(device_coordinates, normalized_depth);
    if (camera_) {
      Transform* transform = camera_->scene_object()->GetComponent<Transform>("Transform");
      return transform ? transform->LocalPositionToWorldSpace(view_space_position) : view_space_position;
    } else {
      return TransformPosition(custom_view_to_world_space_, view_space_position);
    }
  }

  static void RegisterType(sol::table* module);

 protected:
  Matrix3x4 custom_world_to_view_space_;
  Matrix3x4 custom_view_to_world_space_;
  Matrix4 custom_view_to_clip_space_;
  Matrix4 custom_clip_to_view_space_;

 private:
  Scene* scene_ = nullptr;
  Camera* camera_ = nullptr;
};

}  // namespace ovis