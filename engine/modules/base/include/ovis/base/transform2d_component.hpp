#pragma once

#include <ovis/math/transform.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

class Transform2DComponent : public SceneObjectComponent {
 public:
  inline Transform* transform() { return &transform_; }
  inline const Transform* transform() const { return &transform_; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

 private:
  Transform transform_;

  static const json schema;
};

}  // namespace ovis
