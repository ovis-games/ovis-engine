#pragma once

#include <unordered_map>

#include "ovis/input/key.hpp"

namespace ovis {

extern const std::unordered_map<std::string, Key> EMSCRIPTEN_CODE_TO_KEY;

}  // namespace ovis
