#include <ovis/core/vm_bindings.hpp>

namespace ovis {

VirtualMachineBinding VirtualMachineBinding::Create(std::string_view module_name, RegisterFunction* register_function) {
  const auto binding = VirtualMachineBinding{ module_name, register_function};
  bindings().push_back(binding);
  return binding;
}

std::vector<VirtualMachineBinding>& VirtualMachineBinding::bindings() {
  static auto bindings = new std::vector<VirtualMachineBinding>();
  return *bindings;
}

}  // namespace ovis
