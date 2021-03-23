#pragma once

#include <sol/sol.hpp>

#include <ovis/core/event.hpp>
#include <ovis/input/key.hpp>

namespace ovis {

class KeyboardEvent : public Event {
 public:
  inline KeyboardEvent(std::string type, Key key) : Event(std::move(type)), key_(key) {}

  inline Key key() const { return key_; }

 private:
  Key key_;
};

class KeyPressEvent : public KeyboardEvent {
 public:
  inline static const std::string TYPE = "KeyPress";

  inline KeyPressEvent(Key key) : KeyboardEvent(TYPE, key) {}

  static void RegisterType(sol::table* module);
};

class KeyReleaseEvent : public KeyboardEvent {
 public:
  inline static const std::string TYPE = "KeyRelease";

  inline KeyReleaseEvent(Key key) : KeyboardEvent(TYPE, key) {}

  static void RegisterType(sol::table* module);
};

}  // namespace ovis
