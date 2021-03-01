
#include <ovis/core/log.hpp>
#include <ovis/physics2d/physics2d_module.hpp>
#include <ovis/physics2d/physics_world2d.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

Physics2DModule::Physics2DModule() : Module("Physics2D") {
  RegisterSceneController("PhysicsWorld2D", [](Scene*) { return std::make_unique<PhysicsWorld2D>(); });
  RegisterSceneObjectComponent<RigidBody2D>("RigidBody2D", [](SceneObject*) { return std::make_unique<RigidBody2D>(); });
}

Physics2DModule::~Physics2DModule() {}

}  // namespace ovis
