#include <SDL_assert.h>

#include <ovis/rendering2d/shape2d.hpp>

namespace ovis {

const json Shape2D::schema = {{"$ref", "rendering2d#/$defs/shape2d"}};

json Shape2D::Serialize() const {
  json data = {{"color", color_},
               {"outlineColor", outline_color_},
               {"outlineWidth", outline_width_},
               {"texture", texture_asset_}};

  switch (type()) {
    case Type::RECTANGLE:
      data["type"] = "Rectangle";
      data["size"] = rectangle_.size;
      break;

    case Type::ELLIPSE:
      data["type"] = "Ellipse";
      data["size"] = ellipse_.size;
      data["segmentCount"] = ellipse_.num_segments;
      break;
  }

  return data;
}

bool Shape2D::Deserialize(const json& data) {
  if (data.contains("color")) {
    color_ = data.at("color");
  } else {
    color_ = Color::White();
  }
  if (data.contains("outlineColor")) {
    outline_color_ = data.at("outlineColor");
  } else {
    outline_color_ = Color::Black();
  }
  if (data.contains("outlineWidth")) {
    outline_width_ = data.at("outlineWidth");
  } else {
    outline_width_ = 1.0f;
  }
  if (data.contains("texture")) {
    texture_asset_ = data.at("texture");
  } else {
    texture_asset_ = "";
  }

  const std::string& type = data.contains("type") ? data.at("type") : "Rectangle";
  if (type == "Rectangle") {
    type_ = Type::RECTANGLE;
    Rectangle rect;
    if (data.contains("size")) {
      rect.size = data.at("size");
    } else {
      rect.size = { 10.0f, 10.0f};
    }
    SetRectangle(rect);
  } else if (type == "Ellipse") {
    type_ = Type::ELLIPSE;
    Ellipse ellipse;
    if (data.contains("size")) {
      ellipse.size = data.at("size");
    } else {
      ellipse.size = { 10.0f, 10.0f};
    }
    if (data.contains("segmentCount")) {
      ellipse.num_segments = data.at("segmentCount");
    } else {
      ellipse.num_segments = 32;
    }
    SetEllipse(ellipse);
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
  SDL_assert(type_ == Type::RECTANGLE);

  const Vector2 inner_half_size = 0.5f * rectangle_.size + std::min(outline_width_, 0.0f) * Vector2::One();
  const Vector2 outer_half_size = inner_half_size + std::abs(outline_width_) * Vector2::One();
  const uint32_t inner_color = ConvertToRGBA8(color_);
  const uint32_t outline_color = ConvertToRGBA8(outline_color_);
  if (outline_width_ == 0.0f) {
    vertices_ = {
      { -inner_half_size.x, -inner_half_size.y, inner_color },
      {  inner_half_size.x, -inner_half_size.y, inner_color },
      {  inner_half_size.x,  inner_half_size.y, inner_color },
      { -inner_half_size.x, -inner_half_size.y, inner_color },
      {  inner_half_size.x,  inner_half_size.y, inner_color },
      { -inner_half_size.x,  inner_half_size.y, inner_color },
    };
  } else {
    vertices_ = {
      { -inner_half_size.x, -inner_half_size.y, inner_color },
      {  inner_half_size.x, -inner_half_size.y, inner_color },
      {  inner_half_size.x,  inner_half_size.y, inner_color },
      { -inner_half_size.x, -inner_half_size.y, inner_color },
      {  inner_half_size.x,  inner_half_size.y, inner_color },
      { -inner_half_size.x,  inner_half_size.y, inner_color },

      // Top outline
      { -inner_half_size.x,  inner_half_size.y, outline_color },
      {  inner_half_size.x,  inner_half_size.y, outline_color },
      {  outer_half_size.x,  outer_half_size.y, outline_color },

      { -inner_half_size.x,  inner_half_size.y, outline_color },
      {  outer_half_size.x,  outer_half_size.y, outline_color },
      { -outer_half_size.x,  outer_half_size.y, outline_color },

      // Bottom outline
      { -outer_half_size.x, -outer_half_size.y, outline_color },
      {  outer_half_size.x, -outer_half_size.y, outline_color },
      {  inner_half_size.x, -inner_half_size.y, outline_color },

      { -outer_half_size.x, -outer_half_size.y, outline_color },
      {  inner_half_size.x, -inner_half_size.y, outline_color },
      { -inner_half_size.x, -inner_half_size.y, outline_color },

      // Right outline
      {  inner_half_size.x,  inner_half_size.y, outline_color },
      {  inner_half_size.x, -inner_half_size.y, outline_color },
      {  outer_half_size.x, -outer_half_size.y, outline_color },

      {  inner_half_size.x,  inner_half_size.y, outline_color },
      {  outer_half_size.x, -outer_half_size.y, outline_color },
      {  outer_half_size.x,  outer_half_size.y, outline_color },

      // Left outline
      { -inner_half_size.x, -inner_half_size.y, outline_color },
      { -inner_half_size.x,  inner_half_size.y, outline_color },
      { -outer_half_size.x,  outer_half_size.y, outline_color },

      { -inner_half_size.x, -inner_half_size.y, outline_color },
      { -outer_half_size.x,  outer_half_size.y, outline_color },
      { -outer_half_size.x, -outer_half_size.y, outline_color },
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
      vertices_.push_back({previous_position.x, previous_position.y, ellipse_color});
      vertices_.push_back({new_position.x, new_position.y, ellipse_color});
      vertices_.push_back({0.0f, 0.0f, ellipse_color});
      previous_position = new_position;
    }
    vertices_.push_back({previous_position.x, previous_position.y, ellipse_color});
    vertices_.push_back({0.0f, inner_half_size.y, ellipse_color});
    vertices_.push_back({0.0f, 0.0f, ellipse_color});
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

      vertices_.push_back({previous_inner_position.x, previous_inner_position.y, outline_color});
      vertices_.push_back({new_inner_position.x, new_inner_position.y, outline_color});
      vertices_.push_back({new_outer_position.x, new_outer_position.y, outline_color});

      vertices_.push_back({previous_inner_position.x, previous_inner_position.y, outline_color});
      vertices_.push_back({new_outer_position.x, new_outer_position.y, outline_color});
      vertices_.push_back({previous_outer_position.x, previous_outer_position.y, outline_color});

      previous_inner_position = new_inner_position;
      previous_outer_position = new_outer_position;
    }
  }

  SDL_assert(vertices_.size() == ellipse_vertices + outline_vertices);
}

}  // namespace ovis
