#include "ovis/vm/virtual_machine_instructions.hpp"
#include <ovis/vm/script_type_parser.hpp>

namespace ovis {

Result<ParseScriptTypeResult, ParseScriptErrors> ParseScriptType(VirtualMachine* virtual_machine,
                                                                 const json& type_definition,
                                                                 std::string_view script_name,
                                                                 std::string_view base_path) {
  TypeDescription description = {
    .virtual_machine = virtual_machine,
    .memory_layout = {
      .is_constructible = true,
      .alignment_in_bytes = ValueStorage::ALIGNMENT,
      .size_in_bytes = 0,
    }
  };
  ParseScriptErrors errors;
  std::vector<TypePropertyDescription> properties;
  if (const auto& name = type_definition["name"]; name.is_string()) {
    description.name = name;
  } else {
    errors.emplace_back(ScriptErrorLocation(script_name, "{}/name", base_path), "Invalid name");
  }

  FunctionDescription construct_function = {
    .virtual_machine = virtual_machine,
    .inputs = {{ .name = "pointer", .type = virtual_machine->GetTypeId<void*>() }},
    .outputs = {},
    .definition = ScriptFunctionDefinition{},
  };
  ScriptFunctionDefinition& construct_function_definition = std::get<1>(construct_function.definition);

  FunctionDescription copy_function = {
    .virtual_machine = virtual_machine,
    .inputs = {{ .name = "pointer", .type = virtual_machine->GetTypeId<void*>() }},
    .outputs = {},
    .definition = ScriptFunctionDefinition{},
  };
  ScriptFunctionDefinition& copy_function_definition = std::get<1>(copy_function.definition);

  bool every_property_trivially_copyable = true;
  bool every_property_copyable = true;

  FunctionDescription destruct_function = {
    .virtual_machine = virtual_machine,
    .inputs = {{ .name = "pointer", .type = virtual_machine->GetTypeId<void*>() }},
    .outputs = {},
    .definition = ScriptFunctionDefinition{},
  };
  ScriptFunctionDefinition& destruct_function_definition = std::get<1>(destruct_function.definition);

  for (const auto& [property_name, property_definition] : type_definition["properties"].items()) {
    const auto& property_type = virtual_machine->GetType(property_definition.at("type"));
    if (!property_type) {
      errors.emplace_back(ScriptErrorLocation(script_name, "{}/properties/{}", base_path, property_name),
                          "Invalid type for property {}: {}", property_name, property_definition.at("type"));
      continue;
    }
    if (property_type->alignment_in_bytes() > description.memory_layout.alignment_in_bytes) {
      description.memory_layout.alignment_in_bytes = property_type->alignment_in_bytes();
    }
    const std::size_t padding_bytes = (property_type->alignment_in_bytes() - (description.memory_layout.size_in_bytes %
                                                                              property_type->alignment_in_bytes())) %
                                      property_type->alignment_in_bytes();
    description.memory_layout.size_in_bytes += padding_bytes;

    description.properties.push_back({
        .name = property_name,
        .type = property_type->id(),
        .access = TypePropertyDescription::PrimitiveAccess { .offset = description.memory_layout.size_in_bytes }
    });

    // Construct function instructions
    construct_function_definition.instructions.insert(construct_function_definition.instructions.end(), {
      Instruction::CreatePushTrivialStackValue(ExecutionContext::GetInputOffset(0, 0)),
      Instruction::CreateOffsetAddress(ExecutionContext::GetFunctionBaseOffset(0, 1), description.memory_layout.size_in_bytes)
    });
    if (property_type->construct_function()->is_script_function()) {
      construct_function_definition.instructions.insert(construct_function_definition.instructions.end(), {
        Instruction::CreatePrepareScriptFunctionCall(0),
        Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 1)),
        Instruction::CreatePushTrivialConstant(construct_function_definition.constants.size()),
        Instruction::CreateScriptFunctionCall(0, 1)
      });
    } else {
      construct_function_definition.instructions.insert(construct_function_definition.instructions.end(), {
        Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 1)),
        Instruction::CreatePushTrivialConstant(construct_function_definition.constants.size()),
        Instruction::CreateCallNativeFunction(1)
      });
    }
    construct_function_definition.instructions.insert(construct_function_definition.instructions.end(), {
      Instruction::CreatePop(1)
    });
    construct_function_definition.constants.push_back(Value::Create(virtual_machine, property_type->construct_function()->handle()));

    // Copy function instructions
    if (!property_type->description().memory_layout.is_copyable) {
      every_property_copyable = false;
    } else {
      copy_function_definition.instructions.insert(copy_function_definition.instructions.end(), {
        Instruction::CreatePushTrivialStackValue(ExecutionContext::GetInputOffset(0, 0)),
        Instruction::CreateOffsetAddress(ExecutionContext::GetFunctionBaseOffset(0, 2), description.memory_layout.size_in_bytes),
        Instruction::CreatePushTrivialStackValue(ExecutionContext::GetInputOffset(0, 1)),
        Instruction::CreateOffsetAddress(ExecutionContext::GetFunctionBaseOffset(0, 2) + 1, description.memory_layout.size_in_bytes)
      });

      if (property_type->trivially_copyable()) {
        copy_function_definition.instructions.insert(copy_function_definition.instructions.end(), {
          Instruction::CreateMemoryCopy(property_type->size_in_bytes()),
        });
      } else {
        every_property_trivially_copyable = false;
        if (property_type->copy_function()->is_script_function()) {
          copy_function_definition.instructions.insert(copy_function_definition.instructions.end(), {
            Instruction::CreatePrepareScriptFunctionCall(0),
            Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 2)),
            Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 2) + 1),
            Instruction::CreatePushTrivialConstant(copy_function_definition.constants.size()),
            Instruction::CreateScriptFunctionCall(0, 2)
          });
        } else {
          copy_function_definition.instructions.insert(copy_function_definition.instructions.end(), {
            Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 2)),
            Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 2) + 1),
            Instruction::CreatePushTrivialConstant(copy_function_definition.constants.size()),
            Instruction::CreateCallNativeFunction(2)
          });
        }
        copy_function_definition.instructions.insert(copy_function_definition.instructions.end(), {
          Instruction::CreatePop(2)
        });
        copy_function_definition.constants.push_back(Value::Create(virtual_machine, property_type->copy_function()->handle()));
      }
    }

    // Destruct function instructions
    if (!property_type->trivially_destructible()) {
      destruct_function_definition.instructions.insert(destruct_function_definition.instructions.end(), {
        Instruction::CreatePushTrivialStackValue(ExecutionContext::GetInputOffset(0, 0)),
        Instruction::CreateOffsetAddress(ExecutionContext::GetFunctionBaseOffset(0, 1), description.memory_layout.size_in_bytes)
      });
      if (property_type->destruct_function()->is_script_function()) {
        destruct_function_definition.instructions.insert(destruct_function_definition.instructions.end(), {
          Instruction::CreatePrepareScriptFunctionCall(0),
          Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 1)),
          Instruction::CreatePushTrivialConstant(destruct_function_definition.constants.size()),
          Instruction::CreateScriptFunctionCall(0, 1)
        });
      } else {
        destruct_function_definition.instructions.insert(destruct_function_definition.instructions.end(), {
          Instruction::CreatePushTrivialStackValue(ExecutionContext::GetFunctionBaseOffset(0, 1)),
          Instruction::CreatePushTrivialConstant(destruct_function_definition.constants.size()),
          Instruction::CreateCallNativeFunction(1)
        });
      }
      destruct_function_definition.instructions.insert(destruct_function_definition.instructions.end(), {
        Instruction::CreatePop(1)
      });
      destruct_function_definition.constants.push_back(Value::Create(virtual_machine, property_type->destruct_function()->handle()));
    }

    description.memory_layout.size_in_bytes += property_type->size_in_bytes();
  }

  if (description.properties.size() > 0) {
    assert(construct_function_definition.instructions.size() > 0);
  }
  construct_function_definition.instructions.push_back(Instruction::CreateReturn(0));
  description.memory_layout.construct = Function::Create(construct_function);

  description.memory_layout.is_copyable = every_property_copyable;
  if (every_property_copyable && !every_property_trivially_copyable) {
    copy_function_definition.instructions.push_back(Instruction::CreateReturn(0));
    description.memory_layout.copy = Function::Create(copy_function);
  }

  // If there are no destruction instructions the type is trivially destructible
  if (destruct_function_definition.instructions.size() > 0) {
    destruct_function_definition.instructions.push_back(Instruction::CreateReturn(0));
    description.memory_layout.destruct = Function::Create(destruct_function);
  }

  using ResultType = Result<ParseScriptTypeResult, ParseScriptErrors>;
  return errors.size() > 0 ? ResultType(errors) : ParseScriptTypeResult{ description };
}

}  // namespace ovis
