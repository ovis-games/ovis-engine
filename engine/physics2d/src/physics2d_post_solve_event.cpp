#include <ovis/physics2d/physics2d_post_solve_event.hpp>

namespace ovis {

void Physics2DPostSolveEvent::RegisterType(sol::table* module) {
  /// An event that occurs when two objects get in contact.
  // @classmod ovis.physics2d.PostSolve
  sol::usertype<Physics2DPostSolveEvent> pre_solve_event_type = module->new_usertype<Physics2DPostSolveEvent>(
      "PostSolveEvent", sol::no_constructor, sol::base_classes, sol::bases<Event, Physics2DContactEvent>());
}

}  // namespace ovis
