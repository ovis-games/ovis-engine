#include <SDL_assert.h>

#include <ovis/rendering2d/shape2d.hpp>

namespace ovis {

const json Shape2D::schema = {{"$ref", "rendering2d#/$defs/shape2d"}};

json Shape2D::Serialize() const {
  json data = {{"Color", color_}, {"Texture", texture_asset_}};

  switch (type()) {
    case Type::RECT:
      data["Type"] = "Rectangle";
      data["Size"] = rect_.size;
      break;

    case Type::CIRCLE:
      data["Type"] = "Circle";
      data["Size"] = circle_.size;
      data["SegmentCount"] = circle_.num_segments;
      break;
  }

  return data;
}

bool Shape2D::Deserialize(const json& data) {
  if (data.contains("Color")) {
    color_ = data.at("Color");
  } else {
    color_ = Color::White();
  }
  if (data.contains("Texture")) {
    texture_asset_ = data.at("Texture");
  } else {
    texture_asset_ = "";
  }

  const std::string& type = data.contains("Type") ? data["Type"] : "Rectangle";
  if (type == "Rectangle") {
    type_ = Type::RECT;
    Rect rect;
    if (data.contains("Size")) {
      rect.size = data.at("Size");
    } else {
      rect.size = { 10.0f, 10.0f};
    }
    SetRect(rect);
  } else if (type == "Circle") {
    type_ = Type::CIRCLE;
    Circle circle;
    if (data.contains("Size")) {
      circle.size = data.at("Size");
    } else {
      circle.size = { 10.0f, 10.0f};
    }
    if (data.contains("SegmentCount")) {
      circle.num_segments = data.at("SegmentCount");
    } else {
      circle.num_segments = 32;
    }
    SetCircle(circle);
  } else {
    return false;
  }
  return true;
}

void Shape2D::SetColor(const Color& color) {
  color_ = color;
  const uint32_t color_rgba = ConvertToRGBA8(color_);
  for (auto& vertex : vertices_) {
    vertex.color = color_rgba;
  }
}

void Shape2D::SetRect(const Rect& rect) {
  const Vector2 half_size = 0.5f * rect.size;
  const uint32_t color_rgba = ConvertToRGBA8(color_);
  vertices_ = {
    { -half_size.x, -half_size.y, color_rgba },
    {  half_size.x, -half_size.y, color_rgba },
    {  half_size.x,  half_size.y, color_rgba },
    { -half_size.x, -half_size.y, color_rgba },
    {  half_size.x,  half_size.y, color_rgba },
    { -half_size.x,  half_size.y, color_rgba },
  };
  rect_ = rect;
}

void Shape2D::SetCircle(const Circle& circle) {
  const Vector2 half_size = 0.5f * circle.size;
  const uint32_t color_rgba = ConvertToRGBA8(color_);

  vertices_.clear();
  vertices_.reserve(circle.num_segments * 3);

  Vector2 previous_position { 0.0f, half_size.y };
  for (int i = 1; i < circle.num_segments; ++i) {
    const float angle = i * 2.0f * Pi<float>() / circle.num_segments;

    const Vector2 new_position = half_size * Vector2 { std::sin(angle), std::cos(angle) };
    vertices_.push_back({ previous_position.x, previous_position.y, color_rgba });
    vertices_.push_back({ new_position.x, new_position.y, color_rgba });
    vertices_.push_back({ 0.0f, 0.0f, color_rgba });
    previous_position = new_position;
  }
  vertices_.push_back({ previous_position.x, previous_position.y, color_rgba });
  vertices_.push_back({ 0.0f, half_size.y, color_rgba });
  vertices_.push_back({ 0.0f, 0.0f, color_rgba });
  circle_ = circle;
}

}  // namespace ovis
