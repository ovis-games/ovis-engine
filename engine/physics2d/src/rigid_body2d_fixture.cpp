#include <box2d/b2_chain_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <box2d/b2_polygon_shape.h>

#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/physics2d/physics_world2d.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>
#include <ovis/physics2d/rigid_body2d_fixture.hpp>

namespace ovis {

const json RigidBody2DFixture::SCHEMA = {{"$ref", "physics2d#/$defs/rigid_body_2d_fixture"}};

RigidBody2DFixture::RigidBody2DFixture(SceneObject* object) : SceneObjectComponent(object) {
  // shape_ = std::make_unique<b2CircleShape>();
  // shape_->m_radius = 10.0f;
  // fixture_definition_.shape = shape_.get();
  // fixture_definition_.userData.pointer = reinterpret_cast<uintptr_t>(this);
  auto world = object->scene()->GetController<PhysicsWorld2D>();

  if (world == nullptr) {
    LogE("RigidBody2D requires the controller 'PhysicsWorld2D'");
    return;
  }

  b2CircleShape circle_shape;
  circle_shape.m_radius = 10;
  b2FixtureDef fixture_definition;
  fixture_definition.userData.pointer = reinterpret_cast<uintptr_t>(this);
  fixture_definition.shape = &circle_shape;
  Reset(fixture_definition);
}

RigidBody2DFixture::~RigidBody2DFixture() {
  auto world = scene_object()->scene()->GetController<PhysicsWorld2D>();
  if (world != nullptr) {
    world->fixtures_to_create_.erase(
        std::remove(world->fixtures_to_create_.begin(), world->fixtures_to_create_.end(), this),
        world->fixtures_to_create_.end());
  }
}

json RigidBody2DFixture::Serialize() const {
  json data = json::object();

  data["friction"] = friction();
  data["restitution"] = restitution();
  data["restitution_threshold"] = restitution_threshold();
  data["density"] = density();
  data["is_sensor"] = is_sensor();
  data["shape"] = SerializeShape();

  return data;
}

bool RigidBody2DFixture::Deserialize(const json& data) {
  b2FixtureDef fixture_definition;
  fixture_definition.userData.pointer = reinterpret_cast<uintptr_t>(this);

  if (data.contains("friction")) {
    fixture_definition.friction = data.at("friction");
  }
  if (data.contains("restitution")) {
    fixture_definition.restitution = data.at("restitution");
  }
  if (data.contains("restitution_threshold")) {
    fixture_definition.restitutionThreshold = data.at("restitution_threshold");
  }
  if (data.contains("density")) {
    fixture_definition.density = data.at("density");
  }
  if (data.contains("is_sensor")) {
    fixture_definition.isSensor = data.at("is_sensor");
  }

  if (data.contains("shape")) {
    return DeserializeShape(&fixture_definition, data.at("shape"));
  } else {
    return false;
  }
}

void RigidBody2DFixture::RegisterType(sol::table* module) {
  /// A rigid 2D physics body fixture.
  // A fixture attaches a shape to a rigid body with additional properties like friction.
  // @classmod ovis.physics2d.RigidBody2DFixture
  sol::usertype<RigidBody2DFixture> rigid_body2d_type =
      module->new_usertype<RigidBody2DFixture>("RigidBody2DFixture", sol::no_constructor);
}

json RigidBody2DFixture::SerializeShape() const {
  json data = json::object();

  if (shape()->GetType() == b2Shape::e_circle) {
    data["type"] = "circle";
    data["radius"] = static_cast<const b2CircleShape*>(shape())->m_radius;
  }

  return data;
}

bool RigidBody2DFixture::DeserializeShape(b2FixtureDef* fixture_definition, const json& data) {
  const std::string type = data["type"];

  if (type == "circle") {
    b2CircleShape circle_shape;

    if (data.contains("radius")) {
      circle_shape.m_radius = data.at("radius");
    }
    fixture_definition->shape = &circle_shape;
    Reset(*fixture_definition);
  } else {
    b2CircleShape circle_shape;
    circle_shape.m_radius = 5.0f;
    fixture_definition->shape = &circle_shape;
    Reset(*fixture_definition);
  }

  return true;
}

void RigidBody2DFixture::FixtureDefinitionDeleter::operator()(b2FixtureDef* fixture_definition) const {
  RigidBody2DFixture* fixture = reinterpret_cast<RigidBody2DFixture*>(fixture_definition->userData.pointer);
  SDL_assert(fixture != nullptr);

  delete fixture_definition->shape;
  delete fixture_definition;
}

void RigidBody2DFixture::FixtureDeleter::operator()(b2Fixture* fixture) const {
  RigidBody2DFixture* body_fixture = reinterpret_cast<RigidBody2DFixture*>(fixture->GetUserData().pointer);
  SDL_assert(fixture != nullptr);

  b2Body* body = fixture->GetBody();
  SDL_assert(body != nullptr);

  b2World* world = body->GetWorld();
  SDL_assert(world != nullptr);

  if (world->IsLocked()) {
    auto physics2d_world = body_fixture->scene_object()->scene()->GetController<PhysicsWorld2D>();
    SDL_assert(physics2d_world != nullptr);
    physics2d_world->fixtures_to_destroy_.push_back(fixture);
    fixture->GetUserData().pointer = 0;
  } else {
    body->DestroyFixture(fixture);
    LogD("[{}] Deleting fixture: {}", (void*)body_fixture, (void*)fixture);
  }
}

void RigidBody2DFixture::CreateFixture() {
  Reset(*std::get<FixtureDefinitionPointer>(fixture_));
}

void RigidBody2DFixture::DestroyFixture() {
  if (std::holds_alternative<FixtureDefinitionPointer>(fixture_)) {
    return;
  }
  b2Fixture* fixture = std::get<FixturePointer>(fixture_).get();
  b2FixtureDef definition;
  definition.density = fixture->GetDensity();
  definition.friction = fixture->GetFriction();
  definition.isSensor = fixture->IsSensor();
  definition.restitution = fixture->GetRestitution();
  definition.restitutionThreshold = fixture->GetRestitutionThreshold();
  definition.shape = fixture->GetShape();
  definition.userData = fixture->GetUserData();
  SetDefinition(definition);
}

void RigidBody2DFixture::Reset(const b2FixtureDef& definition) {
  SDL_assert(definition.userData.pointer == reinterpret_cast<uintptr_t>(this));

  auto body_component = scene_object()->GetComponent<RigidBody2D>("RigidBody2D");
  if (body_component == nullptr || !std::holds_alternative<b2Body*>(body_component->body_)) {
    SetDefinition(definition);
    return;
  }

  b2Body* body = std::get<b2Body*>(body_component->body_);
  if (body->GetWorld()->IsLocked()) {
    SetDefinition(definition);
  } else {
    fixture_.emplace<FixturePointer>(body->CreateFixture(&definition));
    LogD("[{}] Creating fixture: {}", (void*)this, (void*)std::get<FixturePointer>(fixture_).get());
  }
}

void RigidBody2DFixture::SetDefinition(const b2FixtureDef& definition) {
  SDL_assert(definition.userData.pointer == reinterpret_cast<uintptr_t>(this));
  FixtureDefinitionPointer definition_pointer(new b2FixtureDef(definition));
  definition_pointer->shape = CloneShape(definition.shape);
  auto world = scene_object()->scene()->GetController<PhysicsWorld2D>();
  if (world != nullptr) {
    // Remove all existing entrys for this fixture in the create list
    world->fixtures_to_create_.erase(
        std::remove(world->fixtures_to_create_.begin(), world->fixtures_to_create_.end(), this),
        world->fixtures_to_create_.end());
    world->fixtures_to_create_.push_back(this);
  }
  fixture_.emplace<FixtureDefinitionPointer>(std::move(definition_pointer));
}

b2Shape* RigidBody2DFixture::CloneShape(const b2Shape* shape) {
  if (shape == nullptr) {
    return nullptr;
  }

  switch (shape->GetType()) {
    case b2Shape::e_circle:
      return new b2CircleShape(*static_cast<const b2CircleShape*>(shape));

    case b2Shape::e_chain:
      return new b2ChainShape(*static_cast<const b2ChainShape*>(shape));

    case b2Shape::e_edge:
      return new b2EdgeShape(*static_cast<const b2EdgeShape*>(shape));

    case b2Shape::e_polygon:
      return new b2PolygonShape(*static_cast<const b2PolygonShape*>(shape));

    case b2Shape::e_typeCount:
      SDL_assert(false);
      return nullptr;
  }
}

}  // namespace ovis
