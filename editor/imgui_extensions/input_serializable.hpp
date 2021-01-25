#pragma once

#include <type_traits>
#include <ovis/core/serialize.hpp>
#include <ovis/math/serialize.hpp>
#include "input_json.hpp"

namespace ImGui
{

inline bool InputSerializable(const char* label, ovis::Serializable* object) {
  ovis::json data = object->Serialize();
  ovis::json schema = object->GetSchema() ? *object->GetSchema() : ovis::json{};
  if (InputJson(label, &data, schema)) {
    bool deserialization_successful = object->Deserialize(data);
    SDL_assert(deserialization_successful);
    return true;
  } else {
    return false;
  }
}

} // namespace ImGui
