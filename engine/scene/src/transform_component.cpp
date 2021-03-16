#include <SDL_assert.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <ovis/scene/transform_component.hpp>

namespace ovis {

void TransformComponent::RegisterType(sol::table* module) {
  /// Transforms the scene object.
  // @classmod ovis.scene.TransformComponent
  // @testinginclude <ovis/scene/scene.hpp>
  // @cppsetup ovis::Scene scene;
  // @cppsetup ovis::lua["some_scene"] = &scene;
  // @usage local scene = require "ovis.scene"
  // local some_object = some_scene:add_object("Some Object")
  // local transform = some_object:add_component("Transform")
  sol::usertype<TransformComponent> scene_object_type = module->new_usertype<TransformComponent>("TransformComponent", sol::no_constructor);
}

}  // namespace ovis