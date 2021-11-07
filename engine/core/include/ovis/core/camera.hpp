#pragma once

#include <sol/sol.hpp>

#include <ovis/utils/json.hpp>
#include <ovis/core/math_constants.hpp>
#include <ovis/core/matrix.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/transform.hpp>

namespace ovis {

enum class ProjectionType : std::uint8_t {
  ORTHOGRAPHIC,
  PERSPECTIVE,
};

void to_json(json& data, const ProjectionType& projection_type);
void from_json(const json& data, ProjectionType& projection_type);

class Camera : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  explicit inline Camera(SceneObject* object) : SceneObjectComponent(object) {}

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

  inline Vector3 ViewSpacePositionToClipSpace(Vector3 view_space_coordinates) const {
    return TransformPosition(projection_matrix(), view_space_coordinates);
  }

  inline Vector3 ClipSpacePositionToViewSpace(Vector3 clip_space_coordinates) const {
    return TransformPosition(inverse_projection_matrix(), clip_space_coordinates);
  }

  inline Vector3 WorldSpacePositionToViewSpace(Vector3 world_space_coordinates) const {
    const Transform* transform = scene_object()->GetComponent<Transform>();
    return transform ? transform->WorldDirectionToLocal(world_space_coordinates) : world_space_coordinates;
  }

  inline Vector3 ViewSpacePositionToWorldSpace(Vector3 view_space_coordinates) const {
    const Transform* transform = scene_object()->GetComponent<Transform>();
    return transform ? transform->LocalDirectionToWorld(view_space_coordinates) : view_space_coordinates;
  }

  inline Vector3 WorldSpacePositionToClipSpace(Vector3 world_space_coordinates) const {
    return ViewSpacePositionToClipSpace(WorldSpacePositionToViewSpace(world_space_coordinates));
  }

  inline Vector3 ClipSpacePositionToWorldSpace(Vector3 clip_space_coordinates) const {
    return ViewSpacePositionToWorldSpace(ClipSpacePositionToViewSpace(clip_space_coordinates));
  }

  inline Vector3 WorldSpaceDirectionToViewSpace(Vector3 world_space_direction) const {
    const Transform* transform = scene_object()->GetComponent<Transform>();
    return transform ? transform->WorldDirectionToLocal(world_space_direction) : world_space_direction;
  }

  inline Vector3 ViewSpaceDirectionToWorldSpace(Vector3 view_space_direction) const {
    const Transform* transform = scene_object()->GetComponent<Transform>();
    return transform ? transform->LocalDirectionToWorld(view_space_direction) : view_space_direction;
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
