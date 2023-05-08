#include "ovis/input/mouse_events.hpp"

namespace ovis {

OVIS_VM_DEFINE_TYPE_BINDING(Input, MouseMoveEvent) {
  MouseMoveEvent_type->AddAttribute("Core.Event");

  MouseMoveEvent_type->AddProperty<&MouseMoveEvent::screen_space_position>("screenSpacePosition");
  MouseMoveEvent_type->AddProperty<&MouseMoveEvent::relative_screen_space_position>("relativeScreenSpacePosition");
}

OVIS_VM_DEFINE_TYPE_BINDING(Input, MouseButtonPressEvent) {
  MouseButtonPressEvent_type->AddAttribute("Core.Event");

  MouseButtonPressEvent_type->AddProperty<&MouseButtonPressEvent::screen_space_position>("screenSpacePosition");
  MouseButtonPressEvent_type->AddProperty<&MouseButtonPressEvent::button>("button");
}

OVIS_VM_DEFINE_TYPE_BINDING(Input, MouseButtonReleaseEvent) {
  MouseButtonReleaseEvent_type->AddAttribute("Core.Event");

  MouseButtonReleaseEvent_type->AddProperty<&MouseButtonReleaseEvent::screen_space_position>("screenSpacePosition");
  MouseButtonReleaseEvent_type->AddProperty<&MouseButtonReleaseEvent::button>("button");
}

OVIS_VM_DEFINE_TYPE_BINDING(Input, MouseWheelEvent) {
  MouseWheelEvent_type->AddAttribute("Core.Event");

  MouseWheelEvent_type->AddProperty<&MouseWheelEvent::delta>("delta");
}

}  // namespace ovis
