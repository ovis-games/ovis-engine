#pragma once

#include <sol/sol.hpp>

#include <ovis/core/event.hpp>

namespace ovis {

extern sol::state lua;

class LuaErrorEvent : public Event {
 public:
  inline static const std::string TYPE = "LuaError";

  inline LuaErrorEvent(std::string_view message) : Event(TYPE), message_(message) {}

  std::string_view message() const { return message_; }

 private:
  std::string message_;
};

}  // namespace ovis
