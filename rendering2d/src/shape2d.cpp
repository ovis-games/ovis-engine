#include "ovis/rendering2d/shape2d.hpp"

namespace ovis {

void to_json(json& data, const Shape2D& shape2d) {
  data = {
    {"color", shape2d.color()},
    {"outlineColor", shape2d.outline_color()},
    {"outlineWidth", shape2d.outline_width()},
    {"texture", shape2d.texture_asset()}
  };

  switch (shape2d.type()) {
    case Shape2D::Type::RECTANGLE:
      data["type"] = "Rectangle";
      data["size"] = shape2d.rectangle().size;
      break;

    case Shape2D::Type::ELLIPSE:
      data["type"] = "Ellipse";
      data["size"] = shape2d.ellipse().size;
      data["segmentCount"] = shape2d.ellipse().num_segments;
      break;
  }
}

void from_json(const json& data, Shape2D& shape2d) {
  shape2d.SetColor(data.contains("color") ? data.at("color").get<Color>() : Color::White());
  shape2d.SetOutlineColor(data.contains("outlineColor") ? data.at("outlineColor").get<Color>() : Color::Black());
  shape2d.SetOutlineWidth(data.contains("outlineWidth") ? data.at("outlineWidth").get<float>() : 1.0);
  shape2d.SetTexture(data.contains("texture") ? data.at("texture").get<std::string>() : "");

  const std::string& type = data.contains("type") ? data.at("type") : "Rectangle";
  if (type == "Rectangle") {
    Shape2D::Rectangle rect {
      .size = data.contains("size") ? data.at("size").get<Vector2>() : Vector2 { 10.0f, 10.0f},
    };
    shape2d.SetRectangle(rect);
  } else if (type == "Ellipse") {
    Shape2D::Ellipse ellipse {
      .size = data.contains("size") ? data.at("size").get<Vector2>() : Vector2 { 10.0f, 10.0f},
      .num_segments = data.contains("segmentCount") ? data.at("segmentCount").get<uint32_t>() : 32,
    };
    shape2d.SetEllipse(ellipse);
  } else {
    assert(false);
  }
}

void Shape2D::SetColor(const Color& color) {
  color_ = color;
  Update();
}

void Shape2D::SetOutlineColor(const Color& color) {
  outline_color_ = color;
  Update();
}
void Shape2D::SetOutlineWidth(float width) {
  outline_width_ = width;
  Update();
}

void Shape2D::SetRectangle(const Rectangle& rect) {
  type_ = Type::RECTANGLE;
  rectangle_ = rect;
  UpdateRectangle();
}

void Shape2D::SetEllipse(const Ellipse& ellipse) {
  type_ = Type::ELLIPSE;
  ellipse_ = ellipse;
  UpdateEllipse();
}

void Shape2D::Update() {
  switch (type()) {
    case Type::RECTANGLE:
      UpdateRectangle();
      break;

    case Type::ELLIPSE:
      UpdateEllipse();
      break;
  }
}

void Shape2D::UpdateRectangle() {
  assert(type_ == Type::RECTANGLE);

  const Vector2 inner_half_size = 0.5f * rectangle_.size + std::min(outline_width_, 0.0f) * Vector2::One();
  const Vector2 outer_half_size = inner_half_size + std::abs(outline_width_) * Vector2::One();
  const uint32_t inner_color = ConvertToRGBA8(color_);
  const uint32_t outline_color = ConvertToRGBA8(outline_color_);
  if (outline_width_ == 0.0f) {
    vertices_ = {
      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, inner_color },
      {  inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, inner_color },
      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, inner_color },
      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, inner_color },
      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, inner_color },
      { -inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, inner_color },
    };
  } else {
    vertices_ = {
      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, inner_color },
      {  inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, inner_color },
      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, inner_color },
      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, inner_color },
      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, inner_color },
      { -inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, inner_color },

      // Top outline
      { -inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, outline_color },
      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, outline_color },
      {  outer_half_size.x,  outer_half_size.y, 0.0f, 0.0f, outline_color },

      { -inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, outline_color },
      {  outer_half_size.x,  outer_half_size.y, 0.0f, 0.0f, outline_color },
      { -outer_half_size.x,  outer_half_size.y, 0.0f, 0.0f, outline_color },

      // Bottom outline
      { -outer_half_size.x, -outer_half_size.y, 0.0f, 0.0f, outline_color },
      {  outer_half_size.x, -outer_half_size.y, 0.0f, 0.0f, outline_color },
      {  inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, outline_color },

      { -outer_half_size.x, -outer_half_size.y, 0.0f, 0.0f, outline_color },
      {  inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, outline_color },
      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, outline_color },

      // Right outline
      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, outline_color },
      {  inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, outline_color },
      {  outer_half_size.x, -outer_half_size.y, 0.0f, 0.0f, outline_color },

      {  inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, outline_color },
      {  outer_half_size.x, -outer_half_size.y, 0.0f, 0.0f, outline_color },
      {  outer_half_size.x,  outer_half_size.y, 0.0f, 0.0f, outline_color },

      // Left outline
      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, outline_color },
      { -inner_half_size.x,  inner_half_size.y, 0.0f, 0.0f, outline_color },
      { -outer_half_size.x,  outer_half_size.y, 0.0f, 0.0f, outline_color },

      { -inner_half_size.x, -inner_half_size.y, 0.0f, 0.0f, outline_color },
      { -outer_half_size.x,  outer_half_size.y, 0.0f, 0.0f, outline_color },
      { -outer_half_size.x, -outer_half_size.y, 0.0f, 0.0f, outline_color },
    };
  }
}

void Shape2D::UpdateEllipse() {
  const size_t ellipse_vertices = ellipse_.num_segments * 3;
  const size_t outline_vertices = outline_width_ == 0.0f ? 0 : ellipse_.num_segments * 3 * 2;

  vertices_.clear();
  vertices_.reserve(ellipse_vertices + outline_vertices);

  // Add ellipse vertices
  const Vector2 inner_half_size = 0.5f * ellipse_.size + std::min(outline_width_, 0.0f) * Vector2::One();
  {
    const uint32_t ellipse_color = ConvertToRGBA8(color_);

    Vector2 previous_position{0.0f, inner_half_size.y};
    for (int i = 1; i < ellipse_.num_segments; ++i) {
      const float angle = i * 2.0f * Pi<float>() / ellipse_.num_segments;

      const Vector2 new_position = inner_half_size * Vector2{std::sin(angle), std::cos(angle)};
      vertices_.push_back({previous_position.x, previous_position.y, 0.0f, 0.0f, ellipse_color});
      vertices_.push_back({new_position.x, new_position.y, 0.0f, 0.0f, ellipse_color});
      vertices_.push_back({0.0f, 0.0f, 0.0f, 0.0f, ellipse_color});
      previous_position = new_position;
    }
    vertices_.push_back({previous_position.x, previous_position.y, 0.0f, 0.0f, ellipse_color});
    vertices_.push_back({0.0f, inner_half_size.y, 0.0f, 0.0f, ellipse_color});
    vertices_.push_back({0.0f, 0.0f, 0.0f, 0.0f, ellipse_color});
  }

  // Add outline vertices
  if (outline_width_ != 0.0f) {
    const Vector2 outer_half_size = inner_half_size + std::abs(outline_width_) * Vector2::One();
    const uint32_t outline_color = ConvertToRGBA8(outline_color_);

    Vector2 previous_inner_position{0.0f, inner_half_size.y};
    Vector2 previous_outer_position{0.0f, outer_half_size.y};
    for (int i = 0; i < ellipse_.num_segments; ++i) {
      const float angle = (i + 1) * 2.0f * Pi<float>() / ellipse_.num_segments;

      const Vector2 direction = Vector2{std::sin(angle), std::cos(angle)};
      const Vector2 new_inner_position = inner_half_size * direction;
      const Vector2 new_outer_position = outer_half_size * direction;

      vertices_.push_back({previous_inner_position.x, previous_inner_position.y, 0.0f, 0.0f, outline_color});
      vertices_.push_back({new_inner_position.x, new_inner_position.y, 0.0f, 0.0f, outline_color});
      vertices_.push_back({new_outer_position.x, new_outer_position.y, 0.0f, 0.0f, outline_color});

      vertices_.push_back({previous_inner_position.x, previous_inner_position.y, 0.0f, 0.0f, outline_color});
      vertices_.push_back({new_outer_position.x, new_outer_position.y, 0.0f, 0.0f, outline_color});
      vertices_.push_back({previous_outer_position.x, previous_outer_position.y, 0.0f, 0.0f, outline_color});

      previous_inner_position = new_inner_position;
      previous_outer_position = new_outer_position;
    }
  }

  assert(vertices_.size() == ellipse_vertices + outline_vertices);
}

OVIS_VM_DEFINE_TYPE_BINDING(Rendering2D, Shape2D) {
  Shape2D_type->AddAttribute("Core.EntityComponent");
}

}  // namespace ovis
