#include <ovis/vm/function.hpp>
#include <ovis/vm/module.hpp>

namespace ovis {

// Function::Function()
//     : name_(name), inputs_(inputs), outputs_(outputs) {
//   handle_.native_function = function_pointer;
//   // Check that the two least significant bits are zero. I.e., the function pointer is aligned to a four byte boundary.
//   assert(handle_.is_script_function == 0);
//   assert(handle_.zero == 0);
// }

FunctionDescription FunctionDescription::CreateForNativeFunction(VirtualMachine* virtual_machine,
                                                                 NativeFunction* function_pointer,
                                                                 std::vector<ValueDeclaration> inputs,
                                                                 std::vector<ValueDeclaration> outputs,
                                                                 std::string name) {
  return {
    .virtual_machine = virtual_machine,
    .name = std::move(name),
    .inputs = std::move(inputs),
    .outputs = std::move(outputs),
    .definition = NativeFunctionDefinition {
      .function_pointer = function_pointer
    }
  };
}

Function::Function(FunctionDescription description)
    : virtual_machine_(description.virtual_machine),
      name_(std::move(description.name)),
      inputs_(std::move(description.inputs)),
      outputs_(std::move(description.outputs)) {
  if (description.definition.index() == 0) {
    auto native_definition = std::get<NativeFunctionDefinition>(description.definition);
    handle_ = FunctionHandle::FromNativeFunction(native_definition.function_pointer);
  } else {
    auto script_definition = std::get<ScriptFunctionDefinition>(description.definition);
    const auto constants_offset = virtual_machine()->InsertConstants(script_definition.constants);
    script_definition.instructions.insert(script_definition.instructions.begin(),
                                          Instruction::CreateSetConstantBaseOffset(constants_offset));
    handle_ = FunctionHandle::FromScriptFunction(virtual_machine()->InsertInstructions(script_definition.instructions));
  }
}

bool Function::IsCallableWithArguments(std::span<const TypeId> type_ids) const {
  if (inputs().size() != type_ids.size()) {
    return false;
  }

  for (std::size_t i = 0; i < type_ids.size(); ++i) {
    if (inputs_[i].type != type_ids[i]) {
      return false;
    }
  }

  return true;
}

// std::shared_ptr<Function> Function::Deserialize(const json& data) {
//   if (!data.contains("module")) {
//     return nullptr;
//   }
//   const auto& module_json = data.at("module");
//   if (!module_json.is_string()) {
//     return nullptr;
//   }
//   const std::shared_ptr<Module> module = Module::Get(module_json.get_ref<const std::string&>());
//   if (module == nullptr) {
//     return nullptr;
//   }
//  if (!data.contains("name")) {
//     return nullptr;
//   }
//   const auto& name_json = data.at("name");
//   if (!name_json.is_string()) {
//     return nullptr;
//   }
//   return module->GetFunction(name_json.get_ref<const std::string&>());
// }

std::shared_ptr<Function> Function::Create(FunctionDescription description) {
  return std::make_shared<Function>(std::move(description));
}

}  // namespace ovis
