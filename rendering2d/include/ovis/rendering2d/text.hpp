#pragma once

#include "ovis/core/color.hpp"
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

struct Text {
  Color color = Color::White();
  std::string text = "";
  std::string font = "";

  OVIS_VM_DECLARE_TYPE_BINDING();
};

void to_json(json& data, const Text& text);
void from_json(const json& data, Text& text);

}  // namespace ovis
