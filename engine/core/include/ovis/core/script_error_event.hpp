#pragma once

#include <ovis/core/scripting.hpp>
#include <ovis/core/event.hpp>

namespace ovis {

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

