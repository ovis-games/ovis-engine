
#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/physics2d/physics2d_debug_layer.hpp>
#include <ovis/physics2d/physics2d_events.hpp>
#include <ovis/physics2d/physics2d_module.hpp>
#include <ovis/physics2d/physics_world2d.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

int LoadPhysics2DModule(lua_State* l) {
  sol::state_view state(l);

  /// This module provides 2D rendering components.
  // @module ovis.rendering2d
  // @usage local rendering2d = require('ovis.rendering2d')
  sol::table physics2d_module = state.create_table();

  RigidBody2D::RegisterType(&physics2d_module);
  Physics2DContactEvent::RegisterType(&physics2d_module);
  Physics2DBeginContactEvent::RegisterType(&physics2d_module);
  Physics2DEndContactEvent::RegisterType(&physics2d_module);
  Physics2DPreSolveEvent::RegisterType(&physics2d_module);
  Physics2DPostSolveEvent::RegisterType(&physics2d_module);

  return physics2d_module.push();
}

bool LoadPhysics2DModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();

    SceneObjectComponent::Register("RigidBody2D", [](SceneObject*) { return std::make_unique<RigidBody2D>(); });
    SceneController::Register("PhysicsWorld2D", []() { return std::make_unique<PhysicsWorld2D>(); });
    lua.require("ovis.physics2d", &LoadPhysics2DModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis
