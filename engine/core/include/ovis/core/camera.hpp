#pragma once

#include <sol/sol.hpp>

#include <ovis/utils/json.hpp>
#include <ovis/core/math_constants.hpp>
#include <ovis/core/matrix.hpp>
#include <ovis/core/scene_object_component.hpp>

namespace ovis {

enum class ProjectionType : std::uint8_t {
  ORTHOGRAPHIC,
  PERSPECTIVE,
};

void to_json(json& data, const ProjectionType& projection_type);
void from_json(const json& data, ProjectionType& projection_type);

class Camera : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE(Camera);

 public:
  inline void SetProjectionType(ProjectionType projection_type) {
    projection_type_ = projection_type;
    dirty_ = true;
  }
  inline ProjectionType projection_type() const { return projection_type_; }

  inline void SetVerticalFieldOfView(float vertical_field_of_view) {
    vertical_field_of_view_ = vertical_field_of_view;
    dirty_ = true;
  }
  inline float vertical_field_of_view() const { return vertical_field_of_view_; }

  inline void SetAspectRatio(float aspect_ratio) {
    aspect_ratio_ = aspect_ratio;
    dirty_ = true;
  }
  inline float aspect_ratio() const { return aspect_ratio_; }

  inline void SetNearClipPlane(float near_clip_plane) {
    near_clip_plane_ = near_clip_plane;
    dirty_ = true;
  }
  inline float near_clip_plane() const { return near_clip_plane_; }

  inline void SetFarClipPlane(float far_clip_plane) {
    far_clip_plane_ = far_clip_plane;
    dirty_ = true;
  }
  inline float far_clip_plane() const { return far_clip_plane_; }

  inline Matrix4 projection_matrix() const {
    if (dirty_) {
      CalculateMatrices();
    }
    return projection_matrix_;
  }

  inline Matrix4 inverse_projection_matrix() const {
    if (dirty_) {
      CalculateMatrices();
    }
    return inverse_projection_matrix_;
  }

  inline Vector3 ViewSpaceToNormalizedDeviceCoordinates(Vector3 view_space_position) {
    return TransformPosition(projection_matrix(), view_space_position);
  }

  inline Vector3 NormalizedDeviceCoordinatesToViewSpace(Vector3 ndc) {
    return TransformPosition(inverse_projection_matrix(), ndc);
  }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override;

  static void RegisterType(sol::table* module);

 private:
  mutable Matrix4 projection_matrix_;
  mutable Matrix4 inverse_projection_matrix_;
  float vertical_field_of_view_ = 90.0f * DegreesToRadiansFactor<float>();
  float aspect_ratio_ = 1.0f;
  float near_clip_plane_ = 0.1f;
  float far_clip_plane_ = 1000.0f;
  ProjectionType projection_type_ = ProjectionType::PERSPECTIVE;
  mutable bool dirty_ = true;

  void CalculateMatrices() const;
};

}  // namespace ovis