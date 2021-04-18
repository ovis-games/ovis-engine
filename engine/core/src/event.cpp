#include <ovis/core/event.hpp>

namespace ovis {

void Event::RegisterType(sol::table* module) {
  /// The base class for all events.
  // Each event has a unique type identifier. See @{04-events.md}.
  // @classmod ovis.core.Event
  sol::usertype<Event> event_type = module->new_usertype<Event>("Event", sol::no_constructor);

  /// The type of the event.
  // @field[type=string] type
  event_type["type"] = sol::property(&Event::type);

  /// Checks whether the event is flagged for further propagation.
  // @field[type=bool] is_propagating
  // @see stop_propagation
  event_type["is_propagating"] = sol::property(&Event::is_propagating);

  /// Stops the further propagation of the event.
  // All events are passed to all controllers until they are handled. If you handled the event
  // appropriately and do not want any further processing of the event you should call stop_propagation.
  // @function stop_propagation
  event_type["stop_propagation"] = &Event::StopPropagation;
}

}  // namespace ovis
