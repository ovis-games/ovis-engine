#include "ovis/rendering2d/text.hpp"

namespace ovis {

const json Text::schema = json::object();

Text::Text() : SceneObjectComponent() {}

json Text::Serialize() const {
  json text = json::object();
  text["color"] = color_;
  text["text"] = text_;
  if (font_.size() > 0) {
    text["font"] = font_;
  }
  return text;
}

bool Text::Deserialize(const json& data) {
  // TODO: check format
  if (data.contains("color")) {
    color_ = data.at("color");
  } else {
    color_ = Color::White();
  }

  if (data.contains("text")) {
    text_ = data.at("text");
  } else {
    text_ = "";
  }

  if (data.contains("font")) {
    font_ = data.at("font");
  } else {
    font_ = "";
  }
  return true;
}

OVIS_VM_DEFINE_TYPE_BINDING(Rendering2D, Text, SceneObjectComponent) {
  Text_type->attributes.insert("SceneObjectComponent");
}

}  // namespace ovis
