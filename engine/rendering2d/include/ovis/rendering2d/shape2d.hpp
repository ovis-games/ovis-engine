#pragma once

#include <ovis/core/color.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/vector.hpp>
#include <ovis/graphics/texture2d.hpp>

namespace ovis {

class Shape2D : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  enum class Type {
    RECT,
    CIRCLE,
  };
  struct Rect {
    Vector2 size;
  };
  struct Circle {
    Vector2 size;
    uint32_t num_segments;
  };
  struct Polygon {
    std::vector<Vector2> vertices;
  };
  struct Vertex {
    float x;
    float y;
    uint32_t color;
  };
  static_assert(sizeof(Vertex) == 12);

  explicit inline Shape2D(SceneObject* object) : SceneObjectComponent(object) {}

  Color color() const { return color_; }
  Type type() const { return type_; }
  Rect rect() const { SDL_assert(type_ == Type::RECT); return rect_; }
  Circle circle() const { SDL_assert(type_ == Type::CIRCLE); return circle_; }
  std::string texture_asset() const { return texture_asset_; }

  std::span<const Vertex> vertices() const { return vertices_; }

  void SetColor(const Color& color);
  void SetRect(const Rect& rect);
  void SetCircle(const Circle& circle);
  void SetTexture(const std::string& texture_asset) { texture_asset_ = texture_asset; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

  static void RegisterType(sol::table* module);

 private:
  Color color_ = {1.0, 1.0, 1.0, 1.0};
  Type type_ = Type::RECT;
  union {
    Rect rect_;
    Circle circle_;
  };
  std::string texture_asset_;

  std::vector<Vertex> vertices_;

  static const json schema;
};

}  // namespace ovis
