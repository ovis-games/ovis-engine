#include "ovis/vm/virtual_machine_instructions.hpp"
#include <ovis/vm/script_type_parser.hpp>

namespace ovis {

Result<ParseScriptTypeResult, ParseScriptErrors> ParseScriptType(VirtualMachine* virtual_machine,
                                                                 const json& type_definition) {
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
    errors.emplace_back(fmt::format("Invalid name"), "/name");
  }

  FunctionDescription construct_function = {
    .virtual_machine = virtual_machine,
    .inputs = {{ .name = "pointer", .type = virtual_machine->GetTypeId<void*>() }},
    .outputs = {},
    .definition = ScriptFunctionDefinition{},
  };
  ScriptFunctionDefinition& construct_function_definition = std::get<1>(construct_function.definition);

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
      errors.emplace_back(
          fmt::format("Invalid type for property {}: {}", property_name, property_definition.at("type")),
          fmt::format("/properties/{}", property_name));
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

  construct_function_definition.instructions.push_back(Instruction::CreateReturn(0));
  description.memory_layout.construct = Function::Create(construct_function);

  destruct_function_definition.instructions.push_back(Instruction::CreateReturn(0));
  description.memory_layout.destruct = Function::Create(destruct_function);

  using ResultType = Result<ParseScriptTypeResult, ParseScriptErrors>;
  return errors.size() > 0 ? ResultType(errors) : ParseScriptTypeResult{ description };
}

}  // namespace ovis
