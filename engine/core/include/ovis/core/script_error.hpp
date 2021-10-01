#pragma once

#include <ovis/core/scripting.hpp>
#include <ovis/core/event.hpp>

namespace ovis {

struct ScriptActionReference {
  std::string string;

  static ScriptActionReference Root() { return {.string = "/"}; }
};

inline bool operator==(const ScriptActionReference& lhs, const ScriptActionReference& rhs) {
  return lhs.string == rhs.string;
}

inline ScriptActionReference operator/(const ScriptActionReference& reference, int i) {
  assert(reference.string != "");

  if (reference.string == "/") {
    return ScriptActionReference{.string = reference.string + std::to_string(i)};
  } else {
    return ScriptActionReference{.string = reference.string + "/" + std::to_string(i)};
  }
}

struct ScriptError {
  ScriptActionReference action;
  std::string message;
};

class ScriptErrorEvent : public Event {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  ScriptErrorEvent(std::string_view asset_id, ScriptError error)
      : Event("ScriptErrorEvent"), asset_id_(asset_id), error_(std::move(error)) {}

  std::string_view asset_id() const { return asset_id_; }
  const ScriptError& error() const { return error_; }

 private:
   std::string asset_id_;
   ScriptError error_;
};

}

