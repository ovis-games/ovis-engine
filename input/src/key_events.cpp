#include "ovis/input/key_events.hpp"

namespace ovis {

OVIS_VM_DEFINE_TYPE_BINDING(Input, KeyPressEvent) {
  KeyPressEvent_type->AddAttribute("Core.Event");
  KeyPressEvent_type->AddProperty<&KeyPressEvent::key>("key");
}

OVIS_VM_DEFINE_TYPE_BINDING(Input, KeyReleaseEvent) {
  KeyReleaseEvent_type->AddAttribute("Core.Event");
  KeyReleaseEvent_type->AddProperty<&KeyReleaseEvent::key>("key");
}


}  // namespace ovis
