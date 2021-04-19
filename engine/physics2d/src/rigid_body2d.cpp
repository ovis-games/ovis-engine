#include <box2d/b2_circle_shape.h>
#include <box2d/b2_polygon_shape.h>

#include <ovis/core/math_constants.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

const json RigidBody2D::SCHEMA = {{"$ref", "physics2d#/$defs/box2d_body"}};

RigidBody2D::RigidBody2D() {
  body_definition_.userData.pointer = reinterpret_cast<uintptr_t>(this);

  shape_ = std::make_unique<b2CircleShape>();
  shape_->m_radius = 10.0f;
  fixture_definition_.shape = shape_.get();
  fixture_definition_.userData.pointer = reinterpret_cast<uintptr_t>(this);
}

RigidBody2D::~RigidBody2D() {
  // Don't actually destroy the body, we might be in a callback within Step(). Just set the userdata pointer, so the
  // physics world will delete the object.
  if (body_ != nullptr) {
    body_->GetUserData().pointer = 0;
  }
}

json RigidBody2D::Serialize() const {
  json data = json::object();

  if (auto type = this->type(); type == b2BodyType::b2_staticBody) {
    data["type"] = "static";
  } else if (type == b2BodyType::b2_kinematicBody) {
    data["type"] = "kinematic";
  } else if (type == b2BodyType::b2_dynamicBody) {
    data["type"] = "dynamic";
  }

  data["linear_velocity"] = linear_velocity();
  data["angular_velocity"] = angular_velocity() * RadiansToDegreesFactor<float>();
  data["linear_damping"] = linear_damping();
  data["angular_damping"] = angular_damping();
  data["allow_sleep"] = allow_sleep();
  data["is_awake"] = is_awake();
  data["is_rotation_fixed"] = is_rotation_fixed();
  data["is_bullet"] = is_bullet();
  data["is_enabled"] = is_enabled();
  data["gravity_scale"] = gravity_scale();

  data["friction"] = fixture_definition_.friction;
  data["restitution"] = fixture_definition_.restitution;
  data["restitution_threshold"] = fixture_definition_.restitutionThreshold;
  data["density"] = fixture_definition_.density;
  data["is_sensor"] = fixture_definition_.isSensor;

  data["shape"] = SerializeShape();

  return data;
}

bool RigidBody2D::Deserialize(const json& data) {
  if (data.contains("type")) {
    SetType(StringToType(data["type"].get<std::string>()));
  }
  if (data.contains("linear_velocity")) {
    SetLinearVelocity(data.at("linear_velocity"));
  }
  if (data.contains("angular_velocity")) {
    SetAngularVelocity(static_cast<float>(data.at("angular_velocity")) * DegreesToRadiansFactor<float>());
  }
  if (data.contains("linear_damping")) {
    SetLinearDamping(data.at("linear_damping"));
  }
  if (data.contains("angular_damping")) {
    SetAngularDamping(data.at("angular_damping"));
  }
  if (data.contains("allow_sleep")) {
    SetAllowSleep(data.at("allow_sleep"));
  }
  if (data.contains("is_awake")) {
    SetIsAwake(data.at("is_awake"));
  }
  if (data.contains("is_rotation_fixed")) {
    SetIsRotationFixed(data.at("is_rotation_fixed"));
  }
  if (data.contains("is_bullet")) {
    SetIsBullet(data.at("is_bullet"));
  }
  if (data.contains("is_enabled")) {
    SetIsEnabled(data.at("is_enabled"));
  }
  if (data.contains("gravity_scale")) {
    SetGravityScale(data.at("gravity_scale"));
  }

  if (data.contains("friction")) {
    fixture_definition_.friction = data.at("friction");
  }
  if (data.contains("restitution")) {
    fixture_definition_.restitution = data.at("restitution");
  }
  if (data.contains("restitution_threshold")) {
    fixture_definition_.restitutionThreshold = data.at("restitution_threshold");
  }
  if (data.contains("density")) {
    fixture_definition_.density = data.at("density");
  }
  if (data.contains("is_sensor")) {
    fixture_definition_.isSensor = data.at("is_sensor");
  }

  if (data.contains("shape")) {
    DeserializeShape(data.at("shape"));
  }

  return true;
}

json RigidBody2D::SerializeShape() const {
  json data = json::object();

  if (shape_->GetType() == b2Shape::e_circle) {
    b2CircleShape* circle = static_cast<b2CircleShape*>(shape_.get());

    data["type"] = "circle";
    data["radius"] = circle->m_radius;
  } else if (shape_->GetType() == b2Shape::e_polygon) {
    b2PolygonShape* polygon = static_cast<b2PolygonShape*>(shape_.get());

    data["type"] = "polygon";

    std::vector<Vector2> vertices(polygon->m_count);
    for (int i = 0; i < polygon->m_count; ++i) {
      vertices[i].x = polygon->m_vertices[i].x;
      vertices[i].y = polygon->m_vertices[i].y;
    }

    data["vertices"] = vertices;
  }

  return data;
}

void RigidBody2D::DeserializeShape(const json& data) {
  if (data["type"] == "circle") {
    shape_ = std::make_unique<b2CircleShape>();
    if (data.contains("radius")) {
      shape_->m_radius = data.at("radius");
    }
  } else if (data["type"] == "polygon") {
    auto polygon_shape = std::make_unique<b2PolygonShape>();

    if (data.contains("vertices")) {
      std::vector<Vector2> vertices = data.at("vertices");
      std::vector<b2Vec2> box2d_vertices;

      box2d_vertices.reserve(vertices.size());
      for (const auto& vertex : vertices) {
        box2d_vertices.push_back(b2Vec2(vertex.x, vertex.y));
      }
      polygon_shape->Set(box2d_vertices.data(), box2d_vertices.size());
    } else {
      polygon_shape->SetAsBox(1.0f, 1.0f);
    }

    shape_ = std::move(polygon_shape);
  }

  fixture_definition_.shape = shape_.get();
}

void RigidBody2D::RegisterType(sol::table* module) {
  /// A rigid 2D physics body.
  // @classmod ovis.physics2d.RigidBody2D
  sol::usertype<RigidBody2D> rigid_body2d_type = module->new_usertype<RigidBody2D>("RigidBody2D", sol::no_constructor);

  /// Get the total mass of the body.
  // @field[type=number] mass
  rigid_body2d_type["mass"] = sol::property(&RigidBody2D::mass);

  /// The rotational inertia, usually in kg-m^2.
  // @field[type=number] inertia
  rigid_body2d_type["inertia"] = sol::property(&RigidBody2D::inertia);

  /// Get the total center of mass of the body.
  // @field[type=ovis.core.Vector2] center_of_mass
  rigid_body2d_type["center_of_mass"] = sol::property(&RigidBody2D::center_of_mass);

  /// The linear velocity of the center of mass.
  // @field[type=ovis.core.Vector2] linear_velocity
  rigid_body2d_type["linear_velocity"] = sol::property(&RigidBody2D::linear_velocity, &RigidBody2D::SetLinearVelocity);

  /// The angular velocity in degree/second.
  // @field[type=number] angular_velocity
  rigid_body2d_type["angular_velocity"] = sol::property(
      [](const RigidBody2D& rigid_body) { return rigid_body.angular_velocity() * RadiansToDegreesFactor<float>(); },
      [](RigidBody2D& rigid_body, float omega) {
        rigid_body.SetAngularVelocity(omega * DegreesToRadiansFactor<float>());
      });

  /// The linear damping of the body.
  // Linear damping is use to reduce the linear velocity. The damping parameter can be larger than 1.0f but the damping
  // effect becomes sensitive to the time step when the damping parameter is large. Units are 1/time.
  // @field[type=number] linear_damping
  rigid_body2d_type["linear_damping"] = sol::property(&RigidBody2D::linear_damping, &RigidBody2D::SetLinearDamping);

  /// The angular damping of the body.
  // Angular damping is use to reduce the angular velocity. The damping parameter can be larger than 1.0f but the
  // damping effect becomes sensitive to the time step when the damping parameter is large. Units are 1/time.
  // @field[type=number] angular_damping
  rigid_body2d_type["angular_damping"] = sol::property(&RigidBody2D::angular_damping, &RigidBody2D::SetAngularDamping);

  /// The gravity scale of the body.
  // Scale the gravity applied to this body.
  // @field[type=number] gravity_scale
  rigid_body2d_type["gravity_scale"] = sol::property(&RigidBody2D::gravity_scale, &RigidBody2D::SetGravityScale);

  /// The body type: static, kinematic, or dynamic.
  // * static: zero mass, zero velocity, may be manually moved
  // * kinematic: zero mass, non-zero velocity set by user, moved by solver
  // * dynamic: positive mass, non-zero velocity determined by forces, moved by solver
  // Note: if a dynamic body would have zero mass, the mass is set to one.
  // @field[type=number] type
  rigid_body2d_type["type"] =
      sol::property([](const RigidBody2D& rigid_body) { return TypeToString(rigid_body.type()); },
                    [](RigidBody2D& rigid_body, std::string_view type) { rigid_body.SetType(StringToType(type)); });

  /// Is this body treated like a bullet for continuous collision detection?
  // Is this a fast moving body that should be prevented from tunneling through other moving bodies? Note that all
  // bodies are prevented from tunneling through kinematic and static bodies. This setting is only considered on dynamic
  // bodies.
  // @field[type=bool] is_bullet
  rigid_body2d_type["is_bullet"] = sol::property(&RigidBody2D::is_bullet, &RigidBody2D::SetIsBullet);

  /// Is this body allowed to sleep?
  // Set this flag to false if this body should never fall asleep. Note that this increases CPU usage.
  // @field[type=bool] allow_sleep
  rigid_body2d_type["allow_sleep"] = sol::property(&RigidBody2D::allow_sleep, &RigidBody2D::SetAllowSleep);

  /// The sleep state of the body.
  // A sleeping body has very low CPU cost.
  // @field[type=bool] is_awake
  rigid_body2d_type["is_awake"] = sol::property(&RigidBody2D::is_awake, &RigidBody2D::SetAllowSleep);

  /// Is this body enabled?
  // Allow a body to be disabled. A disabled body is not simulated and cannot be collided with or woken up. If you pass
  // a flag of true, all fixtures will be added to the broad-phase. If you pass a flag of false, all fixtures will be
  // removed from the broad-phase and all contacts will be destroyed. Fixtures and joints are otherwise unaffected. You
  // may continue to create/destroy fixtures and joints on disabled bodies. Fixtures on a disabled body are implicitly
  // disabled and will not participate in collisions, ray-casts, or queries. Joints connected to a disabled body are
  // implicitly disabled.
  // @field[type=bool] is_enabled
  rigid_body2d_type["is_enabled"] = sol::property(&RigidBody2D::is_enabled, &RigidBody2D::SetIsEnabled);

  /// Should this body be prevented from rotating?
  // Useful for characters.
  // @field[type=bool] is_rotation_fixed
  rigid_body2d_type["is_rotation_fixed"] =
      sol::property(&RigidBody2D::is_rotation_fixed, &RigidBody2D::SetIsRotationFixed);

  /// Apply a force to the center of mass.
  // This wakes up the body.
  // @function apply_force
  // @param[type=ovis.core.Vector2] force The force vector in world space in Newtons(N).

  /// Apply a force at a world point.
  // If the force is not applied at the center of mass, it will generate a torque and affect the angular velocity. This
  // wakes up the body.
  // @function apply_force
  // @param[type=ovis.core.Vector2] force The force vector in world space in Newtons(N).
  // @param[type=ovis.core.Vector2] point The world position of the point of application.
  rigid_body2d_type["apply_force"] = sol::overload(sol::resolve<void(Vector2)>(&RigidBody2D::ApplyForce),
                                                   sol::resolve<void(Vector2, Vector2)>(&RigidBody2D::ApplyForce));

  /// Apply a torque.
  // This affects the angular velocity without affecting the linear velocity of the center of mass. This wakes up the
  // body.
  // @function apply_torque
  // @param[type=number] torque Torque about the z-axis (out of the screen), usually in N-m.
  rigid_body2d_type["apply_torque"] = &RigidBody2D::ApplyTorque;

  /// Apply an impulse to the center of mass.
  // This immediately modifies the velocity. This wakes up the body.
  // @function apply_linear_impulse
  // @param[type=ovis.core.Vector2] impulse The world impulse vector, usually in N-seconds or kg-m/s.

  /// Apply an impulse at a point.
  // This immediately modifies the velocity. It also modifies the angular velocity if the point of application is not at
  // the center of mass. This wakes up the body.
  // @function apply_linear_impulse
  // @param[type=ovis.core.Vector2] impulse The world impulse vector, usually in N-seconds or kg-m/s.
  // @param[type=ovis.core.Vector2] point The world position of the point of application.
  rigid_body2d_type["apply_linear_impulse"] =
      sol::overload(sol::resolve<void(Vector2)>(&RigidBody2D::ApplyLinearImpulse),
                    sol::resolve<void(Vector2, Vector2)>(&RigidBody2D::ApplyLinearImpulse));

  /// Apply an angular impulse.
  // This wakes up the body.
  // @function apply_angular_impulse
  // @param[type=number] Impulse the angular impulse in units of kg*m*m/s.
  rigid_body2d_type["apply_angular_impulse"] = &RigidBody2D::ApplyAngularImpulse;
}

}  // namespace ovis
