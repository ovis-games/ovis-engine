#include <vector>

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

namespace {
std::vector<std::function<void(Event* event)>> global_event_callbacks;
}

std::size_t RegisterGlobalEventHandler(std::function<void(Event* event)> callback) {
  for (std::size_t i = 0; i < global_event_callbacks.size(); ++i) {
    if (!global_event_callbacks[i]) {
      global_event_callbacks[i] = callback;
      return i;
    }
  }
  global_event_callbacks.push_back(callback);
  return global_event_callbacks.size() - 1;
}

void DeregisterGlobalEventHandler(std::size_t event_hander_index) {
  assert(event_hander_index < global_event_callbacks.size());

  global_event_callbacks[event_hander_index] = std::function<void(Event* event)>();
}

void PostGlobalEvent(Event* event) {
  for (auto& callback : global_event_callbacks) {
    if (callback) {
      callback(event);
      if (!event->is_propagating()) {
        return;
      }
    }
  }
}

}  // namespace ovis
