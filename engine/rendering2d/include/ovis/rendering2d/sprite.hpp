#pragma once

#include <sol/sol.hpp>

#include <ovis/core/color.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/vector.hpp>
#include <ovis/graphics/texture2d.hpp>

namespace ovis {

class Sprite : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE(Sprite);

 public:
  inline Color color() const { return color_; }
  inline Vector2 size() const { return size_; }
  inline std::string texture_asset() const { return texture_asset_; }

  inline void SetColor(const Color& color) { color_ = color; }
  inline void SetSize(const Vector2& size) { size_ = size; }
  inline void SetTexture(const std::string& texture_asset) { texture_asset_ = texture_asset; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

  void RegisterType(sol::table* module);

 private:
  std::string texture_asset_;
  Vector2 size_;
  Color color_ = {1.0, 1.0, 1.0, 1.0};

  static const json schema;
};

}  // namespace ovis
