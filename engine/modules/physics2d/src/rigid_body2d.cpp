#include <box2d/b2_circle_shape.h>

#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

const json RigidBody2D::SCHEMA = {{"$ref", "physics2d#/$defs/box2d_body"}};

RigidBody2D::RigidBody2D() {
  shape_ = std::make_unique<b2CircleShape>();
  shape_->m_radius = 10.0f;
  fixture_definition_.shape = shape_.get();
}

json RigidBody2D::Serialize() const {
  json data = json::object();

  if (body_definition_.type == b2BodyType::b2_staticBody) {
    data["type"] = "static";
  } else if (body_definition_.type == b2BodyType::b2_kinematicBody) {
    data["type"] = "kinematic";
  } else if (body_definition_.type == b2BodyType::b2_dynamicBody) {
    data["type"] = "dynamic";
  }

  data["linear_velocity"] = glm::vec2(body_definition_.linearVelocity.x, body_definition_.linearVelocity.y);
  data["angular_velocity"] = glm::degrees(body_definition_.angularVelocity);
  data["linear_damping"] = body_definition_.linearDamping;
  data["angular_damping"] = body_definition_.angularDamping;
  data["allow_sleep"] = body_definition_.allowSleep;
  data["awake"] = body_definition_.awake;
  data["fixed_rotation"] = body_definition_.fixedRotation;
  data["bullet"] = body_definition_.bullet;
  data["enabled"] = body_definition_.enabled;
  data["gravity_scale"] = body_definition_.gravityScale;

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
    const std::string type = data["type"];
    if (type == "static") {
      body_definition_.type = b2BodyType::b2_staticBody;
    } else if (type == "kinematic") {
      body_definition_.type = b2BodyType::b2_kinematicBody;
    } else if (type == "dynamic") {
      body_definition_.type = b2BodyType::b2_dynamicBody;
    }
  }

  if (data.contains("linear_velocity")) {
    const glm::vec2 linear_velocity = data.at("linear_velocity");
    body_definition_.linearVelocity.x = linear_velocity.x;
    body_definition_.linearVelocity.y = linear_velocity.y;
  }
  if (data.contains("angular_velocity")) {
    body_definition_.angularVelocity = glm::radians<float>(data.at("angular_velocity"));
  }
  if (data.contains("linear_damping")) {
    body_definition_.linearDamping = data.at("linear_damping");
  }
  if (data.contains("angular_damping")) {
    body_definition_.angularDamping = data.at("angular_damping");
  }
  if (data.contains("allow_sleep")) {
    body_definition_.allowSleep = data.at("allow_sleep");
  }
  if (data.contains("awake")) {
    body_definition_.awake = data.at("awake");
  }
  if (data.contains("fixed_rotation")) {
    body_definition_.fixedRotation = data.at("fixed_rotation");
  }
  if (data.contains("bullet")) {
    body_definition_.bullet = data.at("bullet");
  }
  if (data.contains("enabled")) {
    body_definition_.enabled = data.at("enabled");
  }
  if (data.contains("gravity_scale")) {
    body_definition_.gravityScale = data.at("gravity_scale");
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
  }

  return data;
}

void RigidBody2D::DeserializeShape(const json& data) {
  if (data["type"] == "circle") {
    shape_ = std::make_unique<b2CircleShape>();
    if (data.contains("radius")) {
      shape_->m_radius = data.at("radius");
    }
  }

  fixture_definition_.shape = shape_.get();
}

}  // namespace ovis
