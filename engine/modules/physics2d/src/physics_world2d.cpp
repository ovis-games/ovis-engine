#include <imgui.h>
#include <ovis/base/transform_component.hpp>

#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_object.hpp>
#include <ovis/physics2d/physics_world2d.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

PhysicsWorld2D::PhysicsWorld2D() : SceneController("PhysicsWorld2D"), world_(b2Vec2(0.0f, -9.81f)) {}

void PhysicsWorld2D::Update(std::chrono::microseconds delta_time) {
  std::vector<SceneObject*> scene_objects = scene()->GetSceneObjectsWithComponent("RigidBody2D");

  for (SceneObject* object : scene_objects) {
    TransformComponent* transform = object->GetComponent<TransformComponent>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);

    if (body->body_ != nullptr) {
      if (transform) {
        float roll;
        transform->GetYawPitchRoll(nullptr, nullptr, &roll);
        body->body_->SetTransform(b2Vec2(transform->translation().x, transform->translation().y), glm::radians(roll));
      }
    } else {
      body->body_ = world_.CreateBody(&body->body_definition_);
      SDL_assert(body->body_);
      body->fixture_ = body->body_->CreateFixture(body->shape_.get(), 0.5f);
    }
  }

  world_.Step(1.0f / 60, 6, 6);

  for (SceneObject* object : scene_objects) {
    TransformComponent* transform = object->GetComponent<TransformComponent>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);

    if (transform) {
      b2Vec2 position = body->body_->GetPosition();
      transform->SetTranslation({position.x, position.y, transform->translation().z});

      float yaw, pitch, roll;
      transform->GetYawPitchRoll(&yaw, &pitch, &roll);
      transform->SetYawPitchRoll(yaw, pitch, body->body_->GetAngle());
    }
  }
}

bool PhysicsWorld2D::ProcessEvent(const SDL_Event& event) {
  return false;
}

}  // namespace ovis
