#pragma once

#include <ovis/core/json.hpp>
#include <ovis/math/constants.hpp>
#include <ovis/math/constants.hpp>

namespace ovis {

enum class ProjectionType {
  ORTHOGRAPHIC,
  PERSPECTIVE,
};

void to_json(json& data, const ProjectionType& projection_type);
void from_json(const json& data, ProjectionType& projection_type);

class Camera {
 public:
  inline void SetProjectionType(ProjectionType projection_type) { projection_type_ = projection_type; }
  inline ProjectionType projection_type() const { return projection_type_; }

  inline void SetVerticalFieldOfView(float vertical_field_of_view) { vertical_field_of_view_ = vertical_field_of_view; }
  inline float vertical_field_of_view() const { return vertical_field_of_view_; }

  inline void SetAspectRatio(float aspect_ratio) { aspect_ratio_ = aspect_ratio; }
  inline float aspect_ratio() const { return aspect_ratio_; }

  inline void SetNearClipPlane(float near_clip_plane) { near_clip_plane_ = near_clip_plane; }
  inline float near_clip_plane() const { return near_clip_plane_; }

  inline void SetFarClipPlane(float far_clip_plane) { far_clip_plane_ = far_clip_plane; }
  inline float far_clip_plane() const { return far_clip_plane_; }

  void LookAt(const Vector3& target, const Vector3& up = Vector3::PositiveY());

  Matrix4 CalculateProjectionMatrix() const;

  Vector3 ScreenSpaceToViewSpace(Vector3 screen_space_position);
  Vector3 ViewSpaceToScreenSpace(Vector3 view_space_position);

 private:
  ProjectionType projection_type_ = ProjectionType::PERSPECTIVE;
  float vertical_field_of_view_ = 90.0f * DegreesToRadiansFactor<float>();
  float aspect_ratio_ = 1.0f;
  float near_clip_plane_ = 0.1f;
  float far_clip_plane_ = 1000.0f;
};

void to_json(json& data, const Camera& camera);
void from_json(const json& data, Camera& camera);

}  // namespace ovis