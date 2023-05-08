#pragma once

#include "ovis/core/intersection.hpp"
#include "ovis/core/matrix.hpp"
#include "ovis/core/vector.hpp"

namespace ovis {

struct SceneViewport {
  Matrix3x4 world_to_view;
  Matrix3x4 view_to_world;
  Matrix4 view_to_clip;
  Matrix4 clip_to_view;
  Matrix4 world_to_clip;
  Matrix4 clip_to_world;
  Vector2 dimensions;

  Ray3D CalculateViewRay(Vector2 screen_space_coordinates) {
    const Vector3 origin = ScreenSpacePositionToWorldSpace(Vector3::FromVector2(screen_space_coordinates, 0.0f));
    const Vector3 destination = ScreenSpacePositionToWorldSpace(Vector3::FromVector2(screen_space_coordinates, 1.0f));
    return {origin, destination - origin};
  }

  Vector3 ScreenSpacePositionToClipSpace(Vector3 screen_space_coordinates) const {
    return (2.0f * screen_space_coordinates / Vector3::FromVector2(dimensions - Vector2::One(), 1.0f) -
            Vector3::One()) *
           Vector3{1.0f, -1.0f, 1.0f};
  }

  Vector3 ClipSpacePositionToScreenSpace(Vector3 clip_space_coordinates) const {
    return ((0.5f * Vector3{1.0f, -1.0f, 1.0f} * clip_space_coordinates) + Vector3{0.5f, 0.5f, 0.5f}) *
           Vector3::FromVector2(dimensions - Vector2::One(), 1.0f);
  }

  Vector3 ClipSpacePositionToViewSpace(Vector3 clip_space_coordinates) const {
    return TransformPosition(clip_to_view, clip_space_coordinates);
  }

  Vector3 ViewSpacePositionToClipSpace(Vector3 view_space_coordinates) const {
    return TransformPosition(view_to_clip, view_space_coordinates);
  }

  Vector3 ScreenSpacePositionToViewSpace(Vector3 screen_space_coordinates) const {
    return ClipSpacePositionToViewSpace(ScreenSpacePositionToClipSpace(screen_space_coordinates));
  }

  Vector3 ViewSpacePositionToScreenSpace(Vector3 view_space_coordinates) const {
    return ClipSpacePositionToScreenSpace(ViewSpacePositionToClipSpace(view_space_coordinates));
  }

  Vector3 ViewSpacePositionToWorldSpace(Vector3 view_space_coordinates) const {
    return TransformPosition(view_to_world, view_space_coordinates);
  }

  Vector3 WorldSpacePositionToViewSpace(Vector3 world_space_coordinates) const {
    return TransformPosition(world_to_view, world_space_coordinates);
  }

  Vector3 ClipSpacePositionToWorldSpace(Vector3 clip_space_coordinates) const {
    return ViewSpacePositionToWorldSpace(ClipSpacePositionToViewSpace(clip_space_coordinates));
  }

  Vector3 WorldSpacePositionToClipSpace(Vector3 world_space_coordinates) const {
    return ViewSpacePositionToClipSpace(WorldSpacePositionToViewSpace(world_space_coordinates));
  }

  Vector3 ScreenSpacePositionToWorldSpace(Vector3 screen_space_coordinates) const {
    return ViewSpacePositionToWorldSpace(ScreenSpacePositionToViewSpace(screen_space_coordinates));
  }

  Vector3 WorldSpacePositionToScreenSpace(Vector3 world_space_coordinates) const {
    return ViewSpacePositionToScreenSpace(WorldSpacePositionToViewSpace(world_space_coordinates));
  }

  Vector3 ScreenSpaceDirectionToClipSpace(Vector3 screen_space_direction) const {
    return 2.0f * screen_space_direction / Vector3::FromVector2(dimensions - Vector2::One(), 1.0f) *
           Vector3{1.0f, -1.0f, 1.0f};
  }

  Vector3 ClipSpaceDirectionToScreenSpace(Vector3 clip_space_direction) const {
    return (0.5f * Vector3{1.0f, -1.0f, 1.0f} * clip_space_direction) *
           Vector3::FromVector2(dimensions - Vector2::One(), 1.0f);
  }

  Vector3 ViewSpaceDirectionToWorldSpace(Vector3 view_space_direction) const {
    return TransformDirection(view_to_world, view_space_direction);
  }

  Vector3 WorldSpaceDirectionToViewSpace(Vector3 world_space_direction) const {
    return TransformDirection(world_to_view, world_space_direction);
  }

  OVIS_VM_DECLARE_TYPE_BINDING();
};

}  // namespace ovis
