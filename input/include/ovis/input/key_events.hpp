#pragma once

#include "ovis/core/vm_bindings.hpp"
#include "ovis/input/key.hpp"

namespace ovis {

struct KeyPressEvent {
  Key key;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

struct KeyReleaseEvent {
  Key key;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

}  // namespace ovis
