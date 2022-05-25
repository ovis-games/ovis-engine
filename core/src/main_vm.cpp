#include <ovis/vm/virtual_machine.hpp>
#include <ovis/core/main_vm.hpp>
#include <ovis/core/vm_bindings.hpp>

namespace ovis {
VirtualMachine* main_vm;

void InitializeMainVM() {
  main_vm = new VirtualMachine();

  for (const auto& binding : VirtualMachineBinding::bindings) {
    auto module = main_vm->GetModule(binding.module_name);
    if (!module) {
      module = main_vm->RegisterModule(binding.module_name);
    }
    assert(module);
    binding.register_function(module.get());
  }
}

}  // namespace ovis
