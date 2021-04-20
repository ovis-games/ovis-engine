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
  for (SceneObject* object : scene()->GetSceneObjectsWithComponent("RigidBody2D")) {
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    body->body_ = nullptr;
    body->fixture_ = nullptr;
  }
}

void PhysicsWorld2D::Update(std::chrono::microseconds delta_time) {
  if (delta_time.count() == 0) {
    return;
  }

  {
    std::vector<SceneObject*> scene_objects = scene()->GetSceneObjectsWithComponent("RigidBody2D");
    body_cache_.clear();
    body_cache_.reserve(scene_objects.size());

    for (SceneObject* object : scene_objects) {
      Transform* transform = object->GetComponent<Transform>("Transform");
      RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
      SDL_assert(body != nullptr);

      if (body->body_ == nullptr) {
        body->body_ = world_.CreateBody(&body->body_definition_);
        SDL_assert(body->body_);
        body->fixture_ = body->body_->CreateFixture(&body->fixture_definition_);
      }
      body_cache_.push_back(body->body_);

      if (transform) {
        float roll;
        transform->GetYawPitchRoll(nullptr, nullptr, &roll);
        body->body_->SetTransform(b2Vec2(transform->position().x, transform->position().y), roll);
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
  }

  // Destroy all rigid bodies that belong to objects that have been destroyed within a callback from Step():
  for (b2Body* body : body_cache_) {
    if (body->GetUserData().pointer == 0) {
      world_.DestroyBody(body);
    }
  }

  // Don't use the scene_objects vector from before, as the objects may not be valid anymore
  for (SceneObject* object : scene()->GetSceneObjectsWithComponent("RigidBody2D")) {
    Transform* transform = object->GetComponent<Transform>("Transform");
    RigidBody2D* body = object->GetComponent<RigidBody2D>("RigidBody2D");
    SDL_assert(body != nullptr);

    if (transform != nullptr && body->body_ != nullptr) {
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
