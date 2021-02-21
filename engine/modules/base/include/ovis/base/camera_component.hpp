#pragma once

#include <ovis/engine/camera.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

const json CAMERA_COMPONENT_SCHEMA = {{"$ref", "engine#/$defs/camera"}};

using CameraComponent = SimpleSceneObjectComponent<Camera, &CAMERA_COMPONENT_SCHEMA>;

}  // namespace ovis
