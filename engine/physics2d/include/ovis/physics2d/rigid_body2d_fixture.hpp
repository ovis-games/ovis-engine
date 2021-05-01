#pragma once

#include <box2d/b2_fixture.h>

#include <ovis/utils/safe_pointer.hpp>
#include <ovis/utils/serialize.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/physics2d/box2d.hpp>

namespace ovis {

class RigidBody2DFixture : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();
  friend class RigidBody2D;
  friend class PhysicsWorld2D;

 public:
  RigidBody2DFixture(SceneObject* object);
  ~RigidBody2DFixture();

  inline void SetDensity(float density) {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      std::get<FixturePointer>(fixture_)->SetDensity(density);
    } else {
      std::get<FixtureDefinitionPointer>(fixture_)->definition.density = density;
    }
  }
  inline float density() const {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->GetDensity();
    } else {
      return std::get<FixtureDefinitionPointer>(fixture_)->definition.density;
    }
  }

  inline void SetFriction(float friction) {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      std::get<FixturePointer>(fixture_)->SetFriction(friction);
    } else {
      std::get<FixtureDefinitionPointer>(fixture_)->definition.friction = friction;
    }
  }
  inline float friction() const {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->GetFriction();
    } else {
      return std::get<FixtureDefinitionPointer>(fixture_)->definition.friction;
    }
  }

  inline void SetRestitution(float restitution) {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      std::get<FixturePointer>(fixture_)->SetRestitution(restitution);
    } else {
      std::get<FixtureDefinitionPointer>(fixture_)->definition.restitution = restitution;
    }
  }
  inline float restitution() const {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->GetRestitution();
    } else {
      return std::get<FixtureDefinitionPointer>(fixture_)->definition.restitution;
    }
  }

  inline void SetRestitutionThreshold(float restitution_threshold) {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      std::get<FixturePointer>(fixture_)->SetRestitutionThreshold(restitution_threshold);
    } else {
      std::get<FixtureDefinitionPointer>(fixture_)->definition.restitutionThreshold = restitution_threshold;
    }
  }
  inline float restitution_threshold() const {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->GetRestitutionThreshold();
    } else {
      return std::get<FixtureDefinitionPointer>(fixture_)->definition.restitutionThreshold;
    }
  }

  inline void SetIsSensor(bool is_sensor) {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      std::get<FixturePointer>(fixture_)->SetSensor(is_sensor);
    } else {
      std::get<FixtureDefinitionPointer>(fixture_)->definition.isSensor = is_sensor;
    }
  }
  inline bool is_sensor() const {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->IsSensor();
    } else {
      return std::get<FixtureDefinitionPointer>(fixture_)->definition.isSensor;
    }
  }

  inline b2Shape* shape() {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->GetShape();
    } else {
      SDL_assert(std::get<FixtureDefinitionPointer>(fixture_)->shape.get() != nullptr);
      return std::get<FixtureDefinitionPointer>(fixture_)->shape.get();
    }
  }

  inline const b2Shape* shape() const {
    if (std::holds_alternative<FixturePointer>(fixture_)) [[likely]] {
      return std::get<FixturePointer>(fixture_)->GetShape();
    } else {
      SDL_assert(std::get<FixtureDefinitionPointer>(fixture_)->shape.get() != nullptr);
      return std::get<FixtureDefinitionPointer>(fixture_)->shape.get();
    }
  }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &SCHEMA; }

  static void RegisterType(sol::table* module);

 private:
  struct FixtureDeleter {
    void operator()(b2Fixture* fixture) const;
  };

  struct FixtureDefinition {
    std::unique_ptr<b2Shape> shape;
    b2FixtureDef definition;
  };

  using FixturePointer = std::unique_ptr<b2Fixture, FixtureDeleter>;
  using FixtureDefinitionPointer = std::unique_ptr<FixtureDefinition>;
  std::variant<FixtureDefinitionPointer, FixturePointer> fixture_;

  static const json SCHEMA;

  json SerializeShape() const;
  bool DeserializeShape(b2FixtureDef* fixture_definition, const json& data);

  static inline std::string TypeToString(b2Shape::Type type) {
    switch (type) {
      case b2Shape::Type::e_circle:
        return "circle";
      case b2Shape::Type::e_edge:
        return "edge";
      case b2Shape::Type::e_polygon:
        return "polygon";
      case b2Shape::Type::e_chain:
        return "chain";
      case b2Shape::Type::e_typeCount:
        throw std::runtime_error("Invalid type!");
    }
  }

  static inline b2Shape::Type StringToType(std::string_view string) {
    if (string == "circle") {
      return b2Shape::Type::e_circle;
    } else if (string == "edge") {
      return b2Shape::Type::e_edge;
    } else if (string == "polygon") {
      return b2Shape::Type::e_polygon;
    } else if (string == "chain") {
      return b2Shape::Type::e_chain;
    } else {
      throw std::runtime_error("Invalid type!");
    }
  }

  void CreateFixture();
  void DestroyFixture();
  void Reset(const b2FixtureDef& definition);
  void SetDefinition(const b2FixtureDef& definition);
  static std::unique_ptr<b2Shape> CloneShape(const b2Shape* shape);
};

}  // namespace ovis
