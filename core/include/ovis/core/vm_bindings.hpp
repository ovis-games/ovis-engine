#pragma once

// #include <ovis/vm/module.hpp>
#include <string_view>
#include <vector>

#include "ovis/utils/function.hpp"
#include "ovis/vm/type.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

struct VirtualMachineBinding {
  using RegisterFunction = void(VirtualMachine*, std::string_view module);

  std::string_view module_name;
  RegisterFunction* register_function;

  static VirtualMachineBinding Create(std::string_view module_name, RegisterFunction* register_function);
  static std::vector<VirtualMachineBinding>& bindings();
};

}  // namespace ovis

#define OVIS_UNIQUE_NAME2(base, suffix) base##suffix
#define OVIS_UNIQUE_NAME(base, suffix) OVIS_UNIQUE_NAME2(base, suffix)
// #define OVIS_VM_MODULE_INTERNAL(unique_name, module_name)                            \
//   void unique_name(::ovis::Module* module);                                          \
//   ::ovis::VmAutoReg OVIS_UNIQUE_NAME2(unique_name, reg)(#module_name, &unique_name); \
//   void unique_name(::ovis::Module* module)

// #define OVIS_VM_MODULE(module_name) \
//   OVIS_VM_MODULE_INTERNAL(OVIS_UNIQUE_NAME(Ovis##module_name##Module, __COUNTER__), module_name)

// #define OVIS_DECLARE_TYPE_VM_BINDING() \
//   static const ::ovis::VirtualMachineBinding vm_binding;

// #define OVIS_DEFINE_TYPE_VM_BINDING_INTERNAL(unique_name, type, module_name) \
//   type::vm_binding = 

// #define OVIS_DEFINE_TYPE_VM_BINDING(type, module_name) \
//   OVIS_DEFINE_TYPE_VM_BINDING_INTERNAL(OVIS_UNIQUE_NAME(Ovis##module_name##type, __COUNTER__), type, module_name)

// #define OVIS_VM_BINDING_INTERNAL(unique_name, module_name)

#define OVIS_VM_DECLARE_TYPE_BINDING()                                   \
  static void RegisterType(VirtualMachine* vm, std::string_view module); \
  static VirtualMachineBinding vm_binding;

#define OVIS_VM_DEFINE_TYPE_BINDING_INTERNAL(unique_name, module_name, type)                                 \
  void unique_name(TypeDescription*);                                                                        \
  VirtualMachineBinding type::vm_binding = VirtualMachineBinding::Create(#module_name, &type::RegisterType); \
  void type::RegisterType(VirtualMachine* vm, std::string_view module) {                                     \
    auto type_description = vm->CreateTypeDescription<type>(#type, #module_name);                            \
    unique_name(&type_description);                                                                          \
    vm->RegisterType(type_description);                                                                      \
  }                                                                                                          \
  void unique_name(TypeDescription* type##_type)

#define OVIS_VM_DEFINE_TYPE_BINDING(module_name, type) \
  OVIS_VM_DEFINE_TYPE_BINDING_INTERNAL(OVIS_UNIQUE_NAME(vm_binding_##module_name##type, __COUNTER__), module_name, type)

