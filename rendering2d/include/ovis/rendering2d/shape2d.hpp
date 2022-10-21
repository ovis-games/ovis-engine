#pragma once

#include <span>

#include "ovis/core/color.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/vector.hpp"
#include "ovis/core/vm_bindings.hpp"
#include "ovis/graphics/texture2d.hpp"

namespace ovis {

class Shape2D {
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
    float s;
    float t;
    uint32_t color;
  };
  static_assert(sizeof(Vertex) == 20);

  Color color() const { return color_; }
  Color outline_color() const { return outline_color_; }
  float outline_width() const { return outline_width_; }
  Type type() const { return type_; }
  Rectangle rectangle() const { assert(type_ == Type::RECTANGLE); return rectangle_; }
  Ellipse ellipse() const { assert(type_ == Type::ELLIPSE); return ellipse_; }
  std::string texture_asset() const { return texture_asset_; }

  std::span<const Vertex> vertices() const { return vertices_; }

  void SetColor(const Color& color);
  void SetOutlineColor(const Color& color);
  void SetOutlineWidth(float width);
  void SetRectangle(const Rectangle& rectangle);
  void SetEllipse(const Ellipse& ellipse);
  void SetTexture(const std::string& texture_asset) { texture_asset_ = texture_asset; }

  OVIS_VM_DECLARE_TYPE_BINDING();

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

  void Update();
  void UpdateRectangle();
  void UpdateEllipse();
};

void to_json(json& data, const Shape2D& shape2d);
void from_json(const json& data, Shape2D& shape2d);

}  // namespace ovis
