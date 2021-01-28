#include <SDL2/SDL_assert.h>
#include <glm/gtc/matrix_transform.hpp>

#include <ovis/engine/camera.hpp>

namespace ovis {

void to_json(json& data, const ProjectionType& projection_type) {
  if (projection_type == ProjectionType::ORTHOGRAPHIC) {
    data = "Orthographic";
  } else if (projection_type == ProjectionType::PERSPECTIVE) {
    data = "Perspective";
  } else {
    SDL_assert(false && "Invalid projection type");
  }
}

void from_json(const json& data, ProjectionType& projection_type) {
  if (data == "Orthographic") {
    projection_type = ProjectionType::ORTHOGRAPHIC;
  } else if (data == "Perspective") {
    projection_type = ProjectionType::PERSPECTIVE;
  } else {
    SDL_assert(false && "Invalid projection type");
  }
}

glm::mat4 Camera::CalculateProjectionMatrix() const {
  switch (projection_type_) {
    case ProjectionType::ORTHOGRAPHIC: {
      const float half_height = vertical_field_of_view_ * 0.5f;
      const float half_width = half_height * aspect_ratio_;
      return glm::ortho(-half_width, half_width, -half_height, half_height, near_clip_plane_, far_clip_plane_);
    }

    case ProjectionType::PERSPECTIVE: {
      return glm::perspectiveLH(vertical_field_of_view_, aspect_ratio_, near_clip_plane_, far_clip_plane_);
    }

    default:
      SDL_assert(false && "");
      return glm::mat4{};
  }
}

glm::mat4 Camera::CalculateViewProjectionMatrix() const {
  return CalculateProjectionMatrix() * transform_.CalculateInverseMatrix();
}

void to_json(json& data, const Camera& camera) {
// clang-format off
  data = json{
    {"projectionType", camera.projection_type()},
    {"verticalFieldOfView", camera.vertical_field_of_view()},
    {"aspectRatio", camera.aspect_ratio()},
    {"nearClipPlane", camera.near_clip_plane()},
    {"farClipPlane", camera.far_clip_plane()}
  };
// clang-format on
}

void from_json(const json& data, Camera& camera) {
  camera.SetProjectionType(data.at("projectionType"));
  camera.SetVerticalFieldOfView(data.at("verticalFieldOfView"));
  camera.SetAspectRadio(data.at("aspectRatio"));
  camera.SetNearClipPlane(data.at("nearClipPlane"));
  camera.SetFarClipPlane(data.at("farClipPlane"));
}

}  // namespace ovis