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
  virtual Vector2 GetDimensions() const = 0;

  inline float GetAspectRatio() const {
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

  inline Vector3 ScreenSpacePositionToClipSpace(Vector3 screen_space_coordinates) const {
    return (2.0f * screen_space_coordinates / Vector3::FromVector2(GetDimensions() - Vector2::One(), 1.0f) -
            Vector3::One()) *
           Vector3{1.0f, -1.0f, 1.0f};
  }

  inline Vector3 ClipSpacePositionToScreenSpace(Vector3 clip_space_coordinates) const {
    return ((0.5f * Vector3{1.0f, -1.0f, 1.0f} * clip_space_coordinates) + Vector3{0.5f, 0.5f, 0.5f}) *
           Vector3::FromVector2(GetDimensions() - Vector2::One(), 1.0f);
  }

  inline Vector3 ClipSpacePositionToViewSpace(Vector3 clip_space_coordinates) const {
    return camera_ ? camera_->ClipSpacePositionToViewSpace(clip_space_coordinates)
                   : TransformPosition(custom_clip_to_view_space_, clip_space_coordinates);
  }

  inline Vector3 ViewSpacePositionToClipSpace(Vector3 view_space_coordinates) const {
    return camera_ ? camera_->ViewSpacePositionToClipSpace(view_space_coordinates)
                   : TransformPosition(custom_view_to_clip_space_, view_space_coordinates);
  }

  inline Vector3 ScreenSpacePositionToViewSpace(Vector3 screen_space_coordinates) const {
    return ClipSpacePositionToViewSpace(ScreenSpacePositionToClipSpace(screen_space_coordinates));
  }

  inline Vector3 ViewSpacePositionToScreenSpace(Vector3 view_space_coordinates) const {
    return ClipSpacePositionToScreenSpace(ViewSpacePositionToClipSpace(view_space_coordinates));
  }

  inline Vector3 ViewSpacePositionToWorldSpace(Vector3 view_space_coordinates) const {
    return camera_ ? camera_->ViewSpacePositionToWorldSpace(view_space_coordinates)
                   : TransformPosition(custom_view_to_world_space_, view_space_coordinates);
  }

  inline Vector3 WorldSpacePositionToViewSpace(Vector3 world_space_coordinates) const {
    return camera_ ? camera_->WorldSpacePositionToViewSpace(world_space_coordinates)
                   : TransformPosition(custom_world_to_view_space_, world_space_coordinates);
  }

  inline Vector3 ClipSpacePositionToWorldSpace(Vector3 clip_space_coordinates) const {
    return ViewSpacePositionToWorldSpace(ClipSpacePositionToViewSpace(clip_space_coordinates));
  }

  inline Vector3 WorldSpacePositionToClipSpace(Vector3 world_space_coordinates) const {
    return ViewSpacePositionToClipSpace(WorldSpacePositionToViewSpace(world_space_coordinates));
  }

  inline Vector3 ScreenSpacePositionToWorldSpace(Vector3 screen_space_coordinates) const {
    return ViewSpacePositionToWorldSpace(ScreenSpacePositionToViewSpace(screen_space_coordinates));
  }

  inline Vector3 WorldSpacePositionToScreenSpace(Vector3 world_space_coordinates) const {
    return ViewSpacePositionToScreenSpace(WorldSpacePositionToViewSpace(world_space_coordinates));
  }

  inline Vector3 ScreenSpaceDirectionToClipSpace(Vector3 screen_space_direction) const {
    return 2.0f * screen_space_direction / Vector3::FromVector2(GetDimensions() - Vector2::One(), 1.0f) *
           Vector3{1.0f, -1.0f, 1.0f};
  }

  inline Vector3 ClipSpaceDirectionToScreenSpace(Vector3 clip_space_direction) const {
    return (0.5f * Vector3{1.0f, -1.0f, 1.0f} * clip_space_direction) *
           Vector3::FromVector2(GetDimensions() - Vector2::One(), 1.0f);
  }

  inline Vector3 ClipSpaceDirectionToViewSpace(Vector3 clip_space_direction) const {
    return camera_ ? camera_->ClipSpaceDirectionToViewSpace(clip_space_direction)
                   : TransformDirection(custom_clip_to_view_space_, clip_space_direction);
  }

  inline Vector3 ViewSpaceDirectionToClipSpace(Vector3 view_space_direction) const {
    return camera_ ? camera_->ViewSpaceDirectionToClipSpace(view_space_direction)
                   : TransformDirection(custom_view_to_clip_space_, view_space_direction);
  }

  inline Vector3 ScreenSpaceDirectionToViewSpace(Vector3 screen_space_direction) const {
    return ClipSpaceDirectionToViewSpace(ScreenSpaceDirectionToClipSpace(screen_space_direction));
  }

  inline Vector3 ViewSpaceDirectionToScreenSpace(Vector3 view_space_direction) const {
    return ClipSpaceDirectionToScreenSpace(ViewSpaceDirectionToClipSpace(view_space_direction));
  }

  inline Vector3 ViewSpaceDirectionToWorldSpace(Vector3 view_space_direction) const {
    return camera_ ? camera_->ViewSpaceDirectionToWorldSpace(view_space_direction)
                   : TransformDirection(custom_view_to_world_space_, view_space_direction);
  }

  inline Vector3 WorldSpaceDirectionToViewSpace(Vector3 world_space_direction) const {
    return camera_ ? camera_->WorldSpaceDirectionToViewSpace(world_space_direction)
                   : TransformDirection(custom_world_to_view_space_, world_space_direction);
  }

  inline Vector3 ClipSpaceDirectionToWorldSpace(Vector3 clip_space_direction) const {
    return ViewSpaceDirectionToWorldSpace(ClipSpaceDirectionToViewSpace(clip_space_direction));
  }

  inline Vector3 WorldSpaceDirectionToClipSpace(Vector3 world_space_direction) const {
    return ViewSpaceDirectionToClipSpace(WorldSpaceDirectionToViewSpace(world_space_direction));
  }

  inline Vector3 ScreenSpaceDirectionToWorldSpace(Vector3 screen_space_direction) const {
    return ViewSpaceDirectionToWorldSpace(ScreenSpaceDirectionToViewSpace(screen_space_direction));
  }
  inline Vector3 WorldSpaceDirectionToScreenSpace(Vector3 world_space_direction) const {
    return ViewSpaceDirectionToScreenSpace(WorldSpaceDirectionToViewSpace(world_space_direction));
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