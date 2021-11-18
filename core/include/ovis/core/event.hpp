#pragma once

#include <string>
#include <string_view>

#include <sol/sol.hpp>

#include <ovis/utils/event_handler.hpp>
#include <ovis/utils/lua_reference.hpp>

namespace ovis {

class Event : public DynamicallyLuaReferencableBase {
 public:
  inline Event(std::string_view type) : type_(type) {}
  virtual ~Event() = default;

  inline std::string_view type() const { return type_; }

  inline bool is_propagating() const { return is_propagating_; }
  inline void StopPropagation() { is_propagating_ = false; }

  static void RegisterType(sol::table* module);

 private:
  std::string type_;
  bool is_propagating_ = true;
};

std::size_t RegisterGlobalEventHandler(std::function<void(Event* event)> callback);
void DeregisterGlobalEventHandler(std::size_t event_hander_index);
void PostGlobalEvent(Event* event);

}  // namespace ovis
