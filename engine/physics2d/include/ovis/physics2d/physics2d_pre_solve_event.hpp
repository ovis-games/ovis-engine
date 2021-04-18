#pragma once

#include <ovis/physics2d/physics2d_contact_event.hpp>

namespace ovis {

class Physics2DPreSolveEvent : public Physics2DContactEvent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  inline static const std::string_view TYPE = "Physics2DPreSolve";

  Physics2DPreSolveEvent(b2Contact* contact, const b2Manifold* old_manifold)
      : Physics2DContactEvent(TYPE, contact), old_manifold_(old_manifold) {}

  static void RegisterType(sol::table* module);

 private:
  const b2Manifold* old_manifold_;
};

}  // namespace ovis
