#include <iterator>

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

  data["type"] = TypeToString(shape()->GetType());
  switch (shape()->GetType()) {
    case b2Shape::e_circle:
      data["radius"] = static_cast<const b2CircleShape*>(shape())->m_radius;
      break;

    case b2Shape::e_chain: {
      auto chain_shape = static_cast<const b2ChainShape*>(shape());
      std::vector<Vector2> vertices;
      vertices.reserve(chain_shape->m_count);
      std::transform(chain_shape->m_vertices, chain_shape->m_vertices + chain_shape->m_count,
                     std::back_inserter(vertices), FromBox2DVec2);
      data["vertices"] = vertices;
      data["previous_vertex"] = FromBox2DVec2(chain_shape->m_prevVertex);
      data["next_vertex"] = FromBox2DVec2(chain_shape->m_nextVertex);
      break;
    }

    case b2Shape::e_edge: {
      auto edge_shape = static_cast<const b2EdgeShape*>(shape());
      data["vertices"] = {FromBox2DVec2(edge_shape->m_vertex1), FromBox2DVec2(edge_shape->m_vertex2)};
      data["previous_vertex"] = FromBox2DVec2(edge_shape->m_vertex0);
      data["next_vertex"] = FromBox2DVec2(edge_shape->m_vertex3);
      data["edge_type"] = edge_shape->m_oneSided ? "one-sided" : "two-sided";
      break;
    }

    case b2Shape::e_polygon: {
      auto polygon_shape = static_cast<const b2PolygonShape*>(shape());
      std::vector<Vector2> vertices;
      vertices.reserve(polygon_shape->m_count);
      std::transform(polygon_shape->m_vertices, polygon_shape->m_vertices + polygon_shape->m_count,
                     std::back_inserter(vertices), FromBox2DVec2);
      data["vertices"] = vertices;
      break;
    }

    case b2Shape::e_typeCount:
      break;
  }

  return data;
}

bool RigidBody2DFixture::DeserializeShape(b2FixtureDef* fixture_definition, const json& data) {
  const std::string type = data["type"];

  if (type == "circle") {
    b2CircleShape circle_shape;

    if (data.contains("radius")) {
      circle_shape.m_radius = data.at("radius");
    } else if (data.contains("vertices")) {
      const std::vector<Vector2> vertices = data.at("vertices");
      for (const Vector2& v : vertices) {
        circle_shape.m_radius = std::max(circle_shape.m_radius, Length(v));
      }
    } else {
      circle_shape.m_radius = 1.0f;
    }
    fixture_definition->shape = &circle_shape;
    Reset(*fixture_definition);
  } else if (type == "polygon") {
    b2PolygonShape polygon_shape;

    if (data.contains("vertices")) {
      std::vector<Vector2> vertices = data.at("vertices");
      if (vertices.size() < 3) {
        polygon_shape.SetAsBox(1, 1);
      } else {
        std::vector<b2Vec2> points;
        points.reserve(vertices.size());
        std::transform(vertices.begin(), vertices.end(), std::back_inserter(points), ToBox2DVec2);
        polygon_shape.Set(points.data(), points.size());
      }
    } else if (data.contains("radius")) {
      polygon_shape.SetAsBox(data.at("radius"), data.at("radius"));
    } else {
      polygon_shape.SetAsBox(1, 1);
    }

    fixture_definition->shape = &polygon_shape;
    Reset(*fixture_definition);
  } else if (type == "edge") {
    b2EdgeShape edge_shape;

    if (data.contains("vertices")) {
      const std::vector<Vector2> vertices = data.at("vertices");
      if (vertices.size() < 2) {
        edge_shape.SetTwoSided(b2Vec2(-1.0f, 0.0f), b2Vec2(1.0f, 0.0f));
      } else {
        edge_shape.m_vertex1 = ToBox2DVec2(vertices.front());
        edge_shape.m_vertex2 = ToBox2DVec2(vertices.back());
      }
      if (data.contains("previous_vertex")) {
        edge_shape.m_vertex0 = ToBox2DVec2(data.at("previous_vertex"));
      }
      if (data.contains("next_vertex")) {
        edge_shape.m_vertex3 = ToBox2DVec2(data.at("next_vertex"));
      }
      if (data.contains("edge_type")) {
        edge_shape.m_oneSided = data.at("edge_type").get<std::string>() == "one-sided";
      }
    } else if (data.contains("radius")) {
      const b2Vec2 v(data.at("radius"), 0.0f);
      edge_shape.SetTwoSided(-v, v);
    } else {
      edge_shape.SetTwoSided(b2Vec2(-1.0f, 0.0f), b2Vec2(1.0f, 0.0f));
    }

    fixture_definition->shape = &edge_shape;
    Reset(*fixture_definition);
  } else if (type == "chain") {
    b2ChainShape chain_shape;

    std::vector<b2Vec2> chain_vertices = {{-1.0f, 0.0f}, {1.0f, 0.0f}};
    b2Vec2 prev_vertex = {-2.0f, 0.0f};
    b2Vec2 next_vertex = {2.0f, 0.0f};

    if (data.contains("vertices")) {
      const std::vector<Vector2> vertices = data.at("vertices");
      if (vertices.size() > 2) {
        chain_vertices.clear();
        chain_vertices.reserve(vertices.size());
        std::transform(vertices.begin(), vertices.end(), std::back_inserter(chain_vertices), ToBox2DVec2);
      }
    } else if (data.contains("radius")) {
      const float radius = data.at("radius");
      chain_vertices[0].x *= radius;
      chain_vertices[1].x *= radius;
    }

    if (data.contains("previous_vertex")) {
      prev_vertex = ToBox2DVec2(data.at("previous_vertex"));
    } else {
      SDL_assert(chain_vertices.size() >= 2);
      prev_vertex = chain_vertices[0] - (chain_vertices[1] - chain_vertices[0]);
    }

    if (data.contains("next_vertex")) {
      next_vertex = ToBox2DVec2(data.at("next_vertex"));
    } else {
      SDL_assert(chain_vertices.size() >= 2);
      next_vertex = chain_vertices[chain_vertices.size() - 1] +
                    (chain_vertices[chain_vertices.size() - 1] - chain_vertices[chain_vertices.size() - 2]);
    }

    chain_shape.CreateChain(chain_vertices.data(), chain_vertices.size(), prev_vertex, next_vertex);
    fixture_definition->shape = &chain_shape;
    Reset(*fixture_definition);
  } else {
    b2CircleShape circle_shape;
    circle_shape.m_radius = 5.0f;
    fixture_definition->shape = &circle_shape;
    Reset(*fixture_definition);
  }

  return true;
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
  auto& fixture_definition = std::get<FixtureDefinitionPointer>(fixture_);
  fixture_definition->definition.shape = fixture_definition->shape.get();
  Reset(fixture_definition->definition);
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
  FixtureDefinitionPointer fixture_definition = std::make_unique<FixtureDefinition>();
  fixture_definition->definition = definition;
  fixture_definition->shape = CloneShape(definition.shape);
  auto world = scene_object()->scene()->GetController<PhysicsWorld2D>();
  if (world != nullptr) {
    // Remove all existing entrys for this fixture in the create list
    world->fixtures_to_create_.erase(
        std::remove(world->fixtures_to_create_.begin(), world->fixtures_to_create_.end(), this),
        world->fixtures_to_create_.end());
    world->fixtures_to_create_.push_back(this);
  }
  fixture_.emplace<FixtureDefinitionPointer>(std::move(fixture_definition));
}

std::unique_ptr<b2Shape> RigidBody2DFixture::CloneShape(const b2Shape* shape) {
  if (shape == nullptr) {
    return nullptr;
  }

  switch (shape->GetType()) {
    case b2Shape::e_circle:
      return std::make_unique<b2CircleShape>(*static_cast<const b2CircleShape*>(shape));

    case b2Shape::e_chain:
      return std::make_unique<b2ChainShape>(*static_cast<const b2ChainShape*>(shape));

    case b2Shape::e_edge:
      return std::make_unique<b2EdgeShape>(*static_cast<const b2EdgeShape*>(shape));

    case b2Shape::e_polygon:
      return std::make_unique<b2PolygonShape>(*static_cast<const b2PolygonShape*>(shape));

    case b2Shape::e_typeCount:
      SDL_assert(false);
      return nullptr;
  }
}

}  // namespace ovis
