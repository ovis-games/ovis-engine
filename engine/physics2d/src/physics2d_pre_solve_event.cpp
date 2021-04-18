#include <ovis/physics2d/physics2d_pre_solve_event.hpp>

namespace ovis {

void Physics2DPreSolveEvent::RegisterType(sol::table* module) {
  /// An event that occurs when two objects get in contact.
  // @classmod ovis.physics2d.PreSolveEvent
  sol::usertype<Physics2DPreSolveEvent> pre_solve_event_type = module->new_usertype<Physics2DPreSolveEvent>(
      "PreSolveEvent", sol::no_constructor, sol::base_classes, sol::bases<Event, Physics2DContactEvent>());
}

}  // namespace ovis
