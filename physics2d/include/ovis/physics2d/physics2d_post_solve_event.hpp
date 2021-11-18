#pragma once

#include <box2d/b2_world_callbacks.h>

#include <ovis/physics2d/physics2d_contact_event.hpp>

namespace ovis {

class Physics2DPostSolveEvent : public Physics2DContactEvent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  inline static const std::string_view TYPE = "Physics2DPostSolve";

  Physics2DPostSolveEvent(b2Contact* contact, const b2ContactImpulse* impulse)
      : Physics2DContactEvent(TYPE, contact), impulse_(impulse) {}

  static void RegisterType(sol::table* module);

 private:
  const b2ContactImpulse* impulse_;
};

}  // namespace ovis
