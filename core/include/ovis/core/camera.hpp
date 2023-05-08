#pragma once

#include "ovis/utils/json.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/math_constants.hpp"
#include "ovis/core/matrix.hpp"
#include "ovis/core/transform.hpp"

namespace ovis {

enum class ProjectionType : std::uint8_t {
  ORTHOGRAPHIC,
  PERSPECTIVE,
};

void to_json(json& data, const ProjectionType& projection_type);
void from_json(const json& data, ProjectionType& projection_type);

struct Camera {
  float vertical_field_of_view = 90.0f * DegreesToRadiansFactor<float>();
  float aspect_ratio = 1.0f;
  float near_clip_plane = 0.1f;
  float far_clip_plane = 1000.0f;
  ProjectionType projection_type = ProjectionType::PERSPECTIVE;
};

void to_json(json& data, const Camera& projection_type);
void from_json(const json& data, Camera& projection_type);

struct CameraMatrices {
  Matrix4 projection_matrix = Matrix4::Identity();
  Matrix4 inverse_projection_matrix = Matrix4::Identity();
};

// This should into some scene component:
  // inline Vector3 ViewSpacePositionToClipSpace(Vector3 view_space_coordinates) const {
  //   return TransformPosition(projection_matrix(), view_space_coordinates);
  // }

  // inline Vector3 ClipSpacePositionToViewSpace(Vector3 clip_space_coordinates) const {
  //   return TransformPosition(inverse_projection_matrix(), clip_space_coordinates);
  // }

  // inline Vector3 WorldSpacePositionToViewSpace(const Matrix4& camera_transform, Vector3 world_space_coordinates) const {
  //   const Transform* transform = scene_object()->GetComponent<Transform>();
  //   return transform ? transform->WorldDirectionToLocal(world_space_coordinates) : world_space_coordinates;
  // }

  // inline Vector3 WorldSpacePositionToViewSpace(Vector3 world_space_coordinates) const {
  //   const Transform* transform = scene_object()->GetComponent<Transform>();
  //   return transform ? transform->WorldDirectionToLocal(world_space_coordinates) : world_space_coordinates;
  // }

  // inline Vector3 ViewSpacePositionToWorldSpace(Vector3 view_space_coordinates) const {
  //   const Transform* transform = scene_object()->GetComponent<Transform>();
  //   return transform ? transform->LocalDirectionToWorld(view_space_coordinates) : view_space_coordinates;
  // }

  // inline Vector3 WorldSpacePositionToClipSpace(Vector3 world_space_coordinates) const {
  //   return ViewSpacePositionToClipSpace(WorldSpacePositionToViewSpace(world_space_coordinates));
  // }

  // inline Vector3 ClipSpacePositionToWorldSpace(Vector3 clip_space_coordinates) const {
  //   return ViewSpacePositionToWorldSpace(ClipSpacePositionToViewSpace(clip_space_coordinates));
  // }

  // inline Vector3 WorldSpaceDirectionToViewSpace(Vector3 world_space_direction) const {
  //   const Transform* transform = scene_object()->GetComponent<Transform>();
  //   return transform ? transform->WorldDirectionToLocal(world_space_direction) : world_space_direction;
  // }

  // inline Vector3 ViewSpaceDirectionToWorldSpace(Vector3 view_space_direction) const {
  //   const Transform* transform = scene_object()->GetComponent<Transform>();
  //   return transform ? transform->LocalDirectionToWorld(view_space_direction) : view_space_direction;
  // }


}  // namespace ovis
