#pragma once

#include <string>

#include "ovis/core/vm_bindings.hpp"

namespace ovis {

struct TextInputEvent {
  std::string text;
  
  OVIS_VM_DECLARE_TYPE_BINDING();
};

}  // namespace ovis
