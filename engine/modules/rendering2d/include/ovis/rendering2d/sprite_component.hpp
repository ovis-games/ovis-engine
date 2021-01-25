#pragma once

#include <glm/vec2.hpp>

#include <ovis/core/resource.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

class SpriteComponent : public SceneObjectComponent {
 public:
  inline glm::vec4 color() const { return color_; }
  inline glm::vec2 size() const { return size_; }
  inline std::string texture_asset() const { return texture_asset_; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

 private:
  std::string texture_asset_;
  glm::vec2 size_;
  glm::vec4 color_;

  static const json schema;
};

}  // namespace ovis
