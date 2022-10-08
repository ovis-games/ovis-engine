#pragma once

#include "emscripten/val.h"

namespace ovis {
namespace editor {

inline std::string SerializeValue(const emscripten::val& value) {
  return emscripten::val::global("JSON").call<std::string>("stringify", value);
}

}  // namespace editor
}  // namespace ovis
