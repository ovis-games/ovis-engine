#pragma once

#include <sol/sol.hpp>

#include <ovis/core/event.hpp>
#include <ovis/input/key.hpp>

namespace ovis {

class KeyEvent : public Event {
 public:
  inline static const std::string TYPE = "Key";

  inline KeyEvent(Key key, bool pressed) : Event("Key"), key_(key), pressed_(pressed) {}

  inline Key key() const { return key_; }
  inline bool pressed() const { return pressed_; }
  inline bool released() const { return !pressed_; }

  static void RegisterType(sol::table* module);

 private:
  Key key_;
  bool pressed_;
};

}  // namespace ovis
