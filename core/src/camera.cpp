#include <SDL2/SDL_assert.h>

#include "ovis/core/camera.hpp"
#include "ovis/core/math_constants.hpp"

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

void to_json(json& data, const Camera& camera) {
  data = {{"projectionType", camera.projection_type},
          {"verticalFieldOfView", camera.projection_type == ProjectionType::ORTHOGRAPHIC
                                      ? camera.vertical_field_of_view
                                      : camera.vertical_field_of_view * RadiansToDegreesFactor<float>()},
          {"aspectRatio", camera.aspect_ratio},
          {"nearClipPlane", camera.near_clip_plane},
          {"farClipPlane", camera.far_clip_plane}};
}

void from_json(const json& data, Camera& camera) {
  camera.projection_type = data.at("projectionType");
  camera.vertical_field_of_view =
      camera.projection_type == ProjectionType::ORTHOGRAPHIC
          ? static_cast<float>(data.at("verticalFieldOfView"))
          : static_cast<float>(data.at("verticalFieldOfView")) * DegreesToRadiansFactor<float>();
  camera.aspect_ratio = data.at("aspectRatio");
  camera.near_clip_plane = data.at("nearClipPlane");
  camera.far_clip_plane = data.at("farClipPlane");
}

// void Camera::CalculateMatrices() const {
//   switch (projection_type_) {
//     case ProjectionType::ORTHOGRAPHIC: {
//       const float half_height = vertical_field_of_view_ * 0.5f;
//       const float half_width = half_height * aspect_ratio_;
//       projection_matrix_ = Matrix4::FromOrthographicProjection(-half_width, half_width, -half_height, half_height,
//                                                                near_clip_plane_, far_clip_plane_);
//       break;
//     }

//     case ProjectionType::PERSPECTIVE: {
//       projection_matrix_ =
//           Matrix4::FromPerspectiveProjection(vertical_field_of_view_, aspect_ratio_, near_clip_plane_, far_clip_plane_);
//       break;
//     }

//     default:
//       SDL_assert(false && "Invalid projection type");
//       break;
//   }
//   inverse_projection_matrix_ = Invert(projection_matrix_);
//   dirty_ = false;
// }

}  // namespace ovis
