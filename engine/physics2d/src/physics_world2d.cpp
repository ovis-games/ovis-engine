#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/physics2d/box2d.hpp>
#include <ovis/physics2d/physics2d_events.hpp>
#include <ovis/physics2d/physics_world2d.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

const json PhysicsWorld2D::SCHEMA = {{"$ref", "physics2d#/$defs/physics_world2d"}};

PhysicsWorld2D::PhysicsWorld2D()
    : SceneController("PhysicsWorld2D"), world_(b2Vec2(0.0f, -9.81f)), accumulated_time_(0) {
  world_.SetContactListener(this);
}

PhysicsWorld2D::~PhysicsWorld2D() {
  for (SceneObject* object : scene()->GetSceneObjectsWithComponent("RigidBody2DFixture")) {
    object->RemoveComponent("RigidBody2DFixture");
  }
  for (SceneObject* object : scene()->GetSceneObjectsWithComponent("RigidBody2D")) {
    object->RemoveComponent("RigidBody2D");
  }
}

void PhysicsWorld2D::Update(std::chrono::microseconds delta_time) {
  if (delta_time.count() == 0) {
    return;
  }

  for (SceneObject* object : scene()->GetSceneObjectsWithComponent("RigidBody2D")) {
    Transform* transform = object->GetComponent<Transform>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);
    SDL_assert(std::holds_alternative<b2Body*>(body->body_));

    if (transform) {
      float roll;
      transform->GetLocalYawPitchRoll(nullptr, nullptr, &roll);
      std::get<b2Body*>(body->body_)->SetTransform(ToBox2DVec2(transform->world_position()), roll);
    } else {
      std::get<b2Body*>(body->body_)->SetTransform({0, 0}, 0);

      if (body->type() != b2BodyType::b2_staticBody) {
        LogW(
            "RigidBody2d with non-static type, without transform component attached to the object is not allowed. "
            "Changing type to static!");
        body->SetType(b2BodyType::b2_staticBody);
      }
    }
  }

  accumulated_time_ += delta_time;
  const float step_size = 1.0f / update_rate_;
  const auto step_size_us =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>(step_size));

  while (accumulated_time_ >= step_size_us) {
    world_.Step(step_size, velocity_iterations_, position_iterations_);
    accumulated_time_ -= step_size_us;

    for (RigidBody2D* body : bodies_to_create_) {
      body->CreateInternals(std::get<0>(body->body_).get());
    }
    bodies_to_create_.clear();

    for (RigidBody2DFixture* fixture : fixtures_to_create_) {
      fixture->CreateFixture();
    }
    fixtures_to_create_.clear();

    for (b2Fixture* fixture : fixtures_to_destroy_) {
      fixture->GetBody()->DestroyFixture(fixture);
    }
    fixtures_to_destroy_.clear();

    for (b2Body* body : bodies_to_destroy_) {
      world_.DestroyBody(body);
    }
    bodies_to_destroy_.clear();
  }

  // Don't cache the scene_objects vector from before, as the objects may not be valid anymore
  for (SceneObject* object : scene()->GetSceneObjectsWithComponent("RigidBody2D")) {
    Transform* transform = object->GetComponent<Transform>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);
    SDL_assert(std::holds_alternative<b2Body*>(body->body_));

    if (transform != nullptr) {
      b2Body* box2d_body = std::get<b2Body*>(body->body_);
      b2Vec2 position = box2d_body->GetPosition();
      transform->SetWorldPosition({position.x, position.y, transform->world_position().z});

      float yaw, pitch, roll;
      transform->GetLocalYawPitchRoll(&yaw, &pitch, &roll);
      transform->SetLocalYawPitchRoll(yaw, pitch, box2d_body->GetAngle());
    }
  }
}

const json* PhysicsWorld2D::GetSchema() const {
  return &SCHEMA;
}

json PhysicsWorld2D::Serialize() const {
  json data = json::object();
  data["gravity"] = FromBox2DVec2(world_.GetGravity());
  data["update_rate"] = update_rate_;
  data["velocity_iterations"] = velocity_iterations_;
  data["position_iterations"] = position_iterations_;
  return data;
}

bool PhysicsWorld2D::Deserialize(const json& data) {
  if (data.contains("gravity")) {
    world_.SetGravity(ToBox2DVec2(data.at("gravity")));
  }
  if (data.contains("update_rate")) {
    update_rate_ = data.at("update_rate");
  }
  if (data.contains("velocity_iterations")) {
    velocity_iterations_ = data.at("velocity_iterations");
  }
  if (data.contains("position_iterations")) {
    position_iterations_ = data.at("position_iterations");
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
