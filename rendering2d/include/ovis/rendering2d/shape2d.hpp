#pragma once

#include <span>

#include <ovis/core/color.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/vector.hpp>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/graphics/texture2d.hpp>

namespace ovis {

class Shape2D : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  enum class Type {
    RECTANGLE,
    ELLIPSE,
  };
  struct Rectangle {
    Vector2 size;
  };
  struct Ellipse {
    Vector2 size;
    uint32_t num_segments;
  };
  struct Vertex {
    float x;
    float y;
    uint32_t color;
  };
  static_assert(sizeof(Vertex) == 12);

  explicit inline Shape2D(SceneObject* object) : SceneObjectComponent(object) {Deserialize({});}

  Color color() const { return color_; }
  Color outline_color() const { return outline_color_; }
  float outline_width() const { return outline_width_; }
  Type type() const { return type_; }
  Rectangle rectangle() const { SDL_assert(type_ == Type::RECTANGLE); return rectangle_; }
  Ellipse ellipse() const { SDL_assert(type_ == Type::ELLIPSE); return ellipse_; }
  std::string texture_asset() const { return texture_asset_; }

  std::span<const Vertex> vertices() const { return vertices_; }

  void SetColor(const Color& color);
  void SetRectangle(const Rectangle& rectangle);
  void SetEllipse(const Ellipse& ellipse);
  void SetTexture(const std::string& texture_asset) { texture_asset_ = texture_asset; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

  static void RegisterType(sol::table* module);
  static void RegisterType(vm::Module* module);

 private:
  Color color_ = Color::White();
  Type type_ = Type::RECTANGLE;
  union {
    Rectangle rectangle_;
    Ellipse ellipse_;
  };
  float outline_width_ = 1.0f;
  Color outline_color_ = Color::Black();
  std::string texture_asset_;

  std::vector<Vertex> vertices_;

  static const json schema;

  void Update();
  void UpdateRectangle();
  void UpdateEllipse();
};

}  // namespace ovis
