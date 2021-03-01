#include <ovis/physics2d/rigid_body2d.hpp>
#include <box2d/b2_circle_shape.h>

namespace ovis {

const json RigidBody2D::SCHEMA = {{"$ref", "physics2d#/$defs/rigid_body_2d"}};

RigidBody2D::RigidBody2D() {
  shape_ = std::make_unique<b2CircleShape>();
  shape_->m_radius = 10.0f;
  body_definition_.type = b2BodyType::b2_dynamicBody;
}

json RigidBody2D::Serialize() const {
  json data = json::object();
  return data;
}

bool RigidBody2D::Deserialize(const json& data) {
  return true;
}

}  // namespace ovis
