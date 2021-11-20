#include <ovis/rendering2d/text.hpp>
#include <ovis/core/virtual_machine.hpp>

namespace ovis {

const json Text::schema = json::object();

Text::Text(SceneObject* object) : SceneObjectComponent(object) {}

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
  color_ = data.at("color");
  text_ = data.at("text");
  if (data.contains("font")) {
    font_ = data.at("font");
  } else {
    font_ = "";
  }
  return true;
}

void Text::RegisterType(vm::Module* module) {
  module->RegisterType<Text, SceneObjectComponent>("Text");
}

}
