#include "ovis/input/text_input_event.hpp"

#include "ovis/core/vm_bindings.hpp"

namespace ovis {

OVIS_VM_DEFINE_TYPE_BINDING(Input, TextInputEvent) {
  TextInputEvent_type->AddAttribute("Core.Event");
  TextInputEvent_type->AddProperty<&TextInputEvent::text>("text");
}

}  // namespace ovis
