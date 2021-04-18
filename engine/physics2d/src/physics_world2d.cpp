#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/physics2d/box2d.hpp>
#include <ovis/physics2d/physics2d_events.hpp>
#include <ovis/physics2d/physics_world2d.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

const json PhysicsWorld2D::SCHEMA = {{"$ref", "physics2d#/$defs/physics_world2d"}};

PhysicsWorld2D::PhysicsWorld2D() : SceneController("PhysicsWorld2D"), world_(b2Vec2(0.0f, -9.81f)) {
  world_.SetContactListener(this);
}

void PhysicsWorld2D::Update(std::chrono::microseconds delta_time) {
  std::vector<SceneObject*> scene_objects = scene()->GetSceneObjectsWithComponent("RigidBody2D");

  for (SceneObject* object : scene_objects) {
    Transform* transform = object->GetComponent<Transform>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);

    if (body->body_ == nullptr) {
      body->body_ = world_.CreateBody(&body->body_definition_);
      SDL_assert(body->body_);
      body->fixture_ = body->body_->CreateFixture(&body->fixture_definition_);
    }

    if (transform) {
      float roll;
      transform->GetYawPitchRoll(nullptr, nullptr, &roll);
      body->body_->SetTransform(b2Vec2(transform->position().x, transform->position().y), roll);
    }
  }

  world_.Step(1.0f / 60, 6, 6);

  for (SceneObject* object : scene_objects) {
    Transform* transform = object->GetComponent<Transform>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);

    if (transform) {
      b2Vec2 position = body->body_->GetPosition();
      transform->SetPosition({position.x, position.y, transform->position().z});

      float yaw, pitch, roll;
      transform->GetYawPitchRoll(&yaw, &pitch, &roll);
      transform->SetYawPitchRoll(yaw, pitch, body->body_->GetAngle());
    }
  }
}

const json* PhysicsWorld2D::GetSchema() const {
  return &SCHEMA;
}

json PhysicsWorld2D::Serialize() const {
  json data = json::object();
  data["gravity"] = FromBox2DVec2(world_.GetGravity());
  return data;
}

bool PhysicsWorld2D::Deserialize(const json& data) {
  if (data.contains("gravity")) {
    world_.SetGravity(ToBox2DVec2(data.at("gravity")));
  }
  return true;
}

void PhysicsWorld2D::BeginContact(b2Contact* contact) {
  Physics2DBeginContactEvent event(contact);
  scene()->ProcessEvent(&event);
}

void PhysicsWorld2D::EndContact(b2Contact* contact) {
  Physics2DEndContactEvent event(contact);
  scene()->ProcessEvent(&event);
}

void PhysicsWorld2D::PreSolve(b2Contact* contact, const b2Manifold* old_manifold) {
  Physics2DPreSolveEvent event(contact, old_manifold);
  scene()->ProcessEvent(&event);
}

void PhysicsWorld2D::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
  Physics2DPostSolveEvent event(contact, impulse);
  scene()->ProcessEvent(&event);
}

}  // namespace ovis
