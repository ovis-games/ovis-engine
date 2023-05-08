#include "ovis/rendering2d/text.hpp"

namespace ovis {

void to_json(json& data, const Text& text) {
  data = json::object({
    {"color", text.color},
    {"text", text.text},
  });
  if (text.font.size() > 0) {
    data["font"] = text.font;
  }
}
void from_json(const json& data, Text& text) {
  if (data.contains("color")) {
    text.color = data.at("color");
  } else {
    text.color = Color::White();
  }

  if (data.contains("text")) {
    text.text = data.at("text");
  } else {
    text.text = "";
  }

  if (data.contains("font")) {
    text.font = data.at("font");
  } else {
    text.font = "";
  }
}

OVIS_VM_DEFINE_TYPE_BINDING(Rendering2D, Text) {
  Text_type->AddAttribute("Core.EntityComponent");

  Text_type->AddProperty<&Text::color>("color");
  Text_type->AddProperty<&Text::text>("text");
  Text_type->AddProperty<&Text::font>("font");
}

}  // namespace ovis
