#include <SDL2/SDL_assert.h>

#include <ovis/core/camera.hpp>
#include <ovis/core/math_constants.hpp>

namespace ovis {

namespace {
static const json SCHEMA = {{"$ref", "core#/$defs/camera"}};
}

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

json Camera::Serialize() const {
  // clang-format off
  return {
    {"projectionType", projection_type()},
    {"verticalFieldOfView", vertical_field_of_view() * RadiansToDegreesFactor<float>()},
    {"aspectRatio", aspect_ratio()},
    {"nearClipPlane", near_clip_plane()},
    {"farClipPlane", far_clip_plane()}
  };
  // clang-format on
}

bool Camera::Deserialize(const json& data) {
  SetProjectionType(data.at("projectionType"));
  SetVerticalFieldOfView(static_cast<float>(data.at("verticalFieldOfView")) * DegreesToRadiansFactor<float>());
  SetAspectRatio(data.at("aspectRatio"));
  SetNearClipPlane(data.at("nearClipPlane"));
  SetFarClipPlane(data.at("farClipPlane"));
  return true;
}

const json* Camera::GetSchema() const {
  return &SCHEMA;
}

void Camera::RegisterType(sol::table* module) {
  /// A class that represents a virtual camera.
  // @classmod ovis.core.Camera
  // @base SceneObjectComponent
  // @testinginclude <ovis/core/scene.hpp>
  // @cppsetup ovis::Scene scene;
  // @cppsetup ovis::lua["some_scene"] = &scene;
  // @usage local core = require "ovis.core"
  // local Camera = core.Camera
  // @usage local some_object = some_scene:add_object("Some Camera Object")
  // local camera = some_object:add_component("Camera")
  // camera.vertical_field_of_view = 90
  // camera.aspect_ratio = some_scene.main_viewport.width / some_scene.main_viewport.height
  // camera.near_clip_plane = 0.1
  // camera.far_clip_plane = 1000
  // some_scene.main_viewport.camera = camera
  sol::usertype<Camera> camera_type = module->new_usertype<Camera>("Camera", sol::constructors<Camera()>());

  /// The vertical field of view in degrees.
  // The horizontal field of view is calculated as @{vertical_field_of_view} * @{aspect_ratio}.
  // @field[type=number] vertical_field_of_view
  camera_type["vertical_field_of_view"] = sol::property(
      [](const Camera& camera) { return camera.vertical_field_of_view() * RadiansToDegreesFactor<float>(); },
      [](Camera& camera, float fov) { camera.SetVerticalFieldOfView(fov * DegreesToRadiansFactor<float>()); });

  /// The aspect ratio of the camera.
  // Usually viewport wdith / viewport height.
  // @field[type=number] aspect_ratio
  camera_type["aspect_ratio"] = sol::property(&Camera::aspect_ratio, &Camera::SetAspectRatio);

  /// The distance to the near clip plane.
  // Any objects closer to the camera are not drawn. Must be > 0 for perspective projections.
  // @field[type=number] near_clip_plane
  camera_type["near_clip_plane"] = sol::property(&Camera::near_clip_plane, &Camera::SetNearClipPlane);

  /// The distance to the far clip plane.
  // Any objects further from the camera are not drawn.
  // @field[type=number] far_clip_plane
  camera_type["far_clip_plane"] = sol::property(&Camera::far_clip_plane, &Camera::SetFarClipPlane);
}

void Camera::CalculateMatrices() const {
  switch (projection_type_) {
    case ProjectionType::ORTHOGRAPHIC: {
      const float half_height = vertical_field_of_view_ * 0.5f;
      const float half_width = half_height * aspect_ratio_;
      projection_matrix_ = Matrix4::FromOrthographicProjection(-half_width, half_width, -half_height, half_height,
                                                               near_clip_plane_, far_clip_plane_);
    }

    case ProjectionType::PERSPECTIVE: {
      projection_matrix_ =
          Matrix4::FromPerspectiveProjection(vertical_field_of_view_, aspect_ratio_, near_clip_plane_, far_clip_plane_);
    }

    default:
      SDL_assert(false && "Invalid projection type");
  }
  inverse_projection_matrix_ = Invert(projection_matrix_);
  dirty_ = false;
}

}  // namespace ovis