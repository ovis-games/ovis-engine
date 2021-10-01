#pragma once

#include <any>
#include <functional>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <typeindex>
#include <variant>

#include <SDL_assert.h>
#include <fmt/format.h>

#include <ovis/utils/range.hpp>
#include <ovis/utils/serialize.hpp>

namespace ovis {

// class ScriptContext;

// struct ScriptReference {
//   std::string node_id;
//   std::string output;

//   static std::optional<ScriptReference> Parse(std::string_view reference);
// };

}  // namespace ovis
