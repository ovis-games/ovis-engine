#pragma once

#include <ovis/math/basic_types.hpp>

#include <ovis/core/resource.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

class SpriteComponent : public SceneObjectComponent {
 public:
  inline vector4 color() const { return color_; }
  inline vector2 size() const { return size_; }
  inline std::string texture_asset() const { return texture_asset_; }

  inline void SetColor(const vector4& color) { color_ = color; }
  inline void SetSize(const vector2& size) { size_ = size; }
  inline void SetTexture(const std::string& texture_asset) { texture_asset_ = texture_asset; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

 private:
  std::string texture_asset_;
  vector2 size_;
  vector4 color_ = {1.0, 1.0, 1.0, 1.0};

  static const json schema;
};

}  // namespace ovis
