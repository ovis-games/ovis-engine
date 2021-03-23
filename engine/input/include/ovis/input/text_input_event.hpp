#pragma once

#include <sol/sol.hpp>

#include <ovis/core/event.hpp>
#include <ovis/input/key.hpp>

namespace ovis {

class TextInputEvent : public Event {
 public:
  inline static const std::string TYPE = "TextInput";

  inline TextInputEvent(std::string text) : Event("TextInput"), text_(text) {}

  const std::string& text() const { return text_; }

  static void RegisterType(sol::table* module);

 private:
  std::string text_;
};

}  // namespace ovis
