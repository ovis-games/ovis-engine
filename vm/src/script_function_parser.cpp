#include <deque>

#include <ovis/vm/script_function_parser.hpp>

namespace ovis {

Result<ParseScriptFunctionResult, ParseScriptErrors> ParseScriptFunction(VirtualMachine* virtual_machine, const json& function_definition) {
  ScriptFunctionParser parser(virtual_machine);
  parser.Parse(function_definition);
  if (parser.errors.size() > 0) {
    return parser.errors;
  } else {
    return parser.result;
  }
}

const ScriptFunctionScopeValue* ScriptFunctionScope::GetVariable(std::string_view name) {
  for (const auto& variable : values) {
    if (variable.name == name) {
      return &variable;
    }
  }
  return parent ? parent->GetVariable(name) : nullptr;
}

Result<uint32_t, ParseScriptError> ScriptFunctionScope::AddVariable(TypeId type, std::string_view name) {
  if (name.length() > 0 && GetVariable(name)) {
    return ParseScriptError("Duplicate variable name: {}", name);
  }
  values.push_back(ScriptFunctionScopeValue{
    .type_id = type,
    .name = name.length() > 0 ? std::optional(std::string(name)) : std::nullopt,
    .index = static_cast<uint32_t>(base_index + values.size()),
  });
  return values.back().index;
}


NotNull<ScriptFunctionScopeValue*> ScriptFunctionScope::PushValue(TypeId type) {
  values.push_back({
    .type_id = type,
    .index = static_cast<uint32_t>(values.size()),
  });
  return &values.back();
}

uint32_t ScriptFunctionScope::PopValue() {
  const auto index = values.back().index;
  values.pop_back();
  return index;
}

void ScriptFunctionParser::Parse(const json& json_definition) {
  // TODO: check format
  PushScope();
  if (json_definition.contains("outputs")) {
    ParseOutputs(json_definition["outputs"], "/outputs");
  }
  current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  if (json_definition.contains("inputs")) {
    ParseInputs(json_definition["inputs"], "/inputs");
  }
  ParseStatements(json_definition["statements"], "/statements");
}

void ScriptFunctionParser::ParseOutputs(const json& outputs, std::string_view path) {
  assert(outputs.is_array());
  
  for (const auto& output : outputs) {
    assert(output.contains("name"));
    assert(output.contains("type"));
    const auto& type = output.at("type");
    const std::string& name = output.at("name");
    const auto type_id = virtual_machine->GetTypeId(type);
    current_scope()->AddVariable(type_id, name);
    result.function_description.outputs.push_back({ .name = name, .type = type_id });
  }
}

void ScriptFunctionParser::ParseInputs(const json& inputs, std::string_view path) {
  for (const auto& input : inputs) {
    assert(input.contains("name"));
    assert(input.contains("type"));
    const auto& type = input.at("type");
    const std::string& name = input.at("name");
    const auto type_id = virtual_machine->GetTypeId(type);
    current_scope()->AddVariable(type_id, name);
    result.function_description.inputs.push_back({ .name = name, .type = type_id });
  }
}

void ScriptFunctionParser::ParseStatements(const json& statements_definiton, std::string_view path) {
  for (const auto& [index, statement_definition] : statements_definiton.items()) {
    assert(std::stoi(index) >= 0);
    ParseStatement(statement_definition, fmt::format("{}/{}", path, index));
  }
}

void ScriptFunctionParser::ParseStatement(const json& statement_definiton, std::string_view path) {
  const std::string& id = statement_definiton["id"];
  if (id == "return") {
    ParseReturn(statement_definiton, path);
  } else if (id == "variable_declaration") {
    ParseVariableDeclaration(statement_definiton, path);
  } else if (id == "function_call") {
    // ParseFunctionCall(statement_definiton, path);
  } else {
    errors.emplace_back(fmt::format("Invalid statement id: {}", id), path);
  }
}

void ScriptFunctionParser::ParseReturn(const json& return_definition, std::string_view path) {
  assert(return_definition["id"] == "return");

  const auto& outputs = return_definition["outputs"];

  for (const auto& output : IndexRange(result.function_description.outputs)) {
    if (!outputs.contains(output->name)) {
      errors.emplace_back(path, "Missing return value: {}", output->name);
      continue;
    }

    const auto type = ParseExpression(outputs[output->name], fmt::format("{}/outputs/{}", path, output->name));
    if (type != output->type) {
      errors.emplace_back(path, "Incorrect return type for output {}. Expected {}, got {}", output->name,
                          virtual_machine->GetType(output->type)->name(), virtual_machine->GetType(type)->name());
      continue;
    }
    assert(current_scope()->values.back().type_id == type);
    InsertAssignInstructions(path, virtual_machine->GetType(type), ExecutionContext::GetOutputOffset(output.index()));
  }
  InsertInstructions(path, {
    Instruction::CreateReturn(result.function_description.outputs.size()),
  });
}

void ScriptFunctionParser::ParseVariableDeclaration(const json& variable_declaration, std::string_view path) {
  assert(variable_declaration["id"] == "variable_declaration");
  const auto type = virtual_machine->GetType(variable_declaration.at("type"));
  if (!type) {
    errors.emplace_back(fmt::format("Unknown variable type {}", variable_declaration["type"].dump()), path);
    return;
  }

  const std::string& variable_name = variable_declaration["name"];

}

TypeId ScriptFunctionParser::ParseExpression(const json& expression_definition, std::string_view path) {
  if (expression_definition.is_number()) {
    InsertPushConstantInstructions(path, static_cast<double>(expression_definition));
    assert(current_scope()->values.back().type_id == virtual_machine->GetTypeId<double>());
  } else if (expression_definition.is_string()) {
    InsertPushConstantInstructions(path, static_cast<std::string>(expression_definition));
    assert(current_scope()->values.back().type_id == virtual_machine->GetTypeId<std::string>());
  } else if (expression_definition.is_boolean()) {
    InsertPushConstantInstructions(path, static_cast<bool>(expression_definition));
    assert(current_scope()->values.back().type_id == virtual_machine->GetTypeId<bool>());
  } else {
    errors.emplace_back(path, "Unknown expression: {}", expression_definition.dump());
  }
  return current_scope()->values.back().type_id;
}

// void ScriptFunctionParser::ParseFunctionCall(const json& statement_definiton, std::string_view path) {
//   assert(statement_definiton["id"] == "function_call");
//   const auto function = Function::Deserialize(statement_definiton["function"]);
//   if (!function) {
//     errors.emplace_back(fmt::format("Unknown function: {}", statement_definiton["function"].dump(), path));
//     return;
//   }
  
//   for (const auto& input : function->inputs()) {
//   }
// }

// void ScriptFunctionParser::ParsePushValue(const json& value_definition, std::string_view path, TypeId type) {
//   if (value_definition.is_string()) {
//     if (type == virtual_machine->GetTypeId<std::string>()) {
//       errors.emplace_back(path, "Parsing constant Core.String not implemented yet");
//     } else {
//       const auto requested_type = virtual_machine->GetType(type);
//       errors.emplace_back(path, fmt::format("Expected {}, got {}", requested_type ? requested_type->GetReferenceString() : "Unknown", "Core.String"));
//     }
//   } else if (value_definition.is_number()) {
//     if (type == virtual_machine->GetTypeId<double>()) {
//       errors.emplace_back("Parsing constant Core.Number not implemented yet");
//     } else {
//       const auto requested_type = virtual_machine->GetType(type);
//       errors.emplace_back(fmt::format("Expected {}, got {}", requested_type ? requested_type->GetReferenceString() : "Unknown", "Core.Number"));
//     }
//   } else if (value_definition.is_boolean()) {
//     if (type == virtual_machine->GetTypeId<bool>()) {
//       errors.emplace_back("Parsing constant Core.Number not implemented yet", path);
//     } else {
//       const auto requested_type = virtual_machine->GetType(type);
//       errors.emplace_back(fmt::format("Expected {}, got {}", requested_type ? requested_type->GetReferenceString() : "Unknown", "Core.Boolean"));
//     }
//   } else if (value_definition.is_object()) {
//     const std::string& id = value_definition["id"];
//     if (id == "variable") {
//       ParsePushVariable(value_definition, path, type);
//     } else {
//       errors.emplace_back("Invalid value definition", path);
//     }
//   } else {
//     errors.emplace_back("Invalid value definition", path);
//   }
// }

// void ScriptFunctionParser::ParsePushVariable(const json& value_definition, std::string_view path, TypeId type) {
//   assert(value_definition["id"] == "variable");
// }

ScriptFunctionScope* ScriptFunctionParser::PushScope() {
  if (scopes.size() == 0) {
    scopes.push_back({
      .parent = nullptr,
      .base_index = 0,
    });
  } else {
    scopes.push_back({
      .parent = current_scope(),
      .base_index = static_cast<uint32_t>(current_scope()->base_index + current_scope()->values.size()),
    });
  }
  return &scopes.back();
}

void ScriptFunctionParser::PopScope() {
  // Last scope cannot be popped!
  assert(current_scope()->parent);
  InsertInstructions("", { Instruction::CreatePop(current_scope()->values.size()) });
  scopes.pop_back();
}

ScriptFunctionScope* ScriptFunctionParser::current_scope() {
  return &scopes.back();
}

template <typename T>
std::uint32_t ScriptFunctionParser::InsertConstant(T&& value) {
  const auto offset = definition.constants.size();
  definition.constants.push_back(Value::Create(virtual_machine, value));
  return offset;
}

template <typename T>
void ScriptFunctionParser::InsertPushConstantInstructions(std::string_view path, T&& value) {
  const Type* type = virtual_machine->GetType<T>();
  if (!type->memory_layout().is_copyable) {
    errors.emplace_back(path, "{} is not copyable", type->GetReferenceString());
    return;
  }
  
  const auto constant_index = InsertConstant(std::forward<T>(value));
  const auto temporary = current_scope()->PushValue(type->id());

  if (type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreatePushTrivialConstant(constant_index)
    });
  } else if (!type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreatePushAllocated(type->alignment_in_bytes(), type->size_in_bytes()),
      Instruction::CreatePushStackValueAllocatedAddress(temporary->index),
      Instruction::CreatePushConstantAllocatedAddress(constant_index),
      Instruction::CreateMemoryCopy(type->size_in_bytes()),
    });
  } else {
    InsertInstructions(path, {
      type->is_stored_inline()
        ? Instruction::CreatePush(1)
        : Instruction::CreatePushAllocated(type->alignment_in_bytes(), type->alignment_in_bytes())
    });
    InsertPrepareFunctionCallInstructions(path, type->copy_function());
    if (type->is_stored_inline()) {
      InsertInstructions(path, {
        Instruction::CreatePushStackValueDataAddress(temporary->index),
        Instruction::CreatePushConstantDataAddress(constant_index),
      });
    } else {
      InsertInstructions(path, {
        Instruction::CreatePushStackValueAllocatedAddress(temporary->index),
        Instruction::CreatePushConstantAllocatedAddress(constant_index),
      });
    }
    InsertFunctionCallInstructions(path, type->copy_function());
  }
  assert(current_scope()->current_stack_offset() == temporary->index + 1);
}

ScriptFunctionScopeValue* ScriptFunctionParser::InsertConstructTypeInstruction(std::string_view path, NotNull<const Type*> type) {
  if (!type->memory_layout().is_constructible) {
    errors.emplace_back(path, "{} is not constructible", type->GetReferenceString());
    return nullptr;
  }
  const auto construct_function = type->construct_function();
  assert(construct_function);

  auto value = current_scope()->PushValue(type->id());

  InsertInstructions(path, {
    type->is_stored_inline()
      ? Instruction::CreatePush(1)
      : Instruction::CreatePushAllocated(type->alignment_in_bytes(), type->alignment_in_bytes())
  });
  InsertPrepareFunctionCallInstructions(path, construct_function);
  InsertInstructions(path, {
    type->is_stored_inline()
      ? Instruction::CreatePushStackValueDataAddress(value->index)
      : Instruction::CreatePushStackValueAllocatedAddress(value->index),
  });
  InsertFunctionCallInstructions(path, construct_function);

  return value;
}

void ScriptFunctionParser::InsertAssignInstructions(std::string_view path, NotNull<const Type*> type, uint32_t destination_index) {
  assert(current_scope()->values.back().type_id == type->id());

  if (!type->memory_layout().is_copyable) {
    errors.emplace_back(path, "{} is not copyable", type->GetReferenceString());
    return;
  }
  
  if (type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreateAssignTrivial(destination_index),
    });
  } else if (!type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreatePushStackValueAllocatedAddress(destination_index),
      Instruction::CreatePushStackValueAllocatedAddress(current_scope()->values.back().index),
      Instruction::CreateMemoryCopy(type->size_in_bytes()),
    });
  } else {
    InsertPrepareFunctionCallInstructions(path, type->copy_function());
    if (type->is_stored_inline()) {
      InsertInstructions(path, {
      Instruction::CreatePushStackValueDataAddress(destination_index),
      Instruction::CreatePushStackValueDataAddress(current_scope()->values.back().index),
      });
    } else {
      InsertInstructions(path, {
      Instruction::CreatePushStackValueAllocatedAddress(destination_index),
      Instruction::CreatePushStackValueAllocatedAddress(current_scope()->values.back().index),
      });
    }
    InsertFunctionCallInstructions(path, type->copy_function());
  }
  current_scope()->PopValue();
}

void ScriptFunctionParser::InsertPrepareFunctionCallInstructions(std::string_view path, NotNull<const Function*> function) {
  if (function->is_script_function()) {
    for (const auto& output : function->outputs()) {
      InsertConstructTypeInstruction(path, virtual_machine->GetType(output.type));
    }
    InsertInstructions(path, {
      Instruction::CreatePrepareScriptFunctionCall(function->outputs().size()),
    });
    current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
    current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
    current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  }
}

void ScriptFunctionParser::InsertFunctionCallInstructions(std::string_view path, NotNull<const Function*> function) {
  if (function->is_script_function()) {
    InsertPushConstantInstructions(path, function->handle());
    InsertInstructions(path, {
      Instruction::CreateScriptFunctionCall(function->outputs().size(), function->inputs().size()),
    });
    current_scope()->PopValue(); // function handle
    for (const auto& input : function->inputs()) {
      current_scope()->PopValue();
    }
    current_scope()->PopValue(); // return address
    current_scope()->PopValue(); // stack offset
    current_scope()->PopValue(); // constant offset
  } else {
    InsertPushConstantInstructions(path, function->handle());
    InsertInstructions(path, {
      Instruction::CreateScriptFunctionCall(function->outputs().size(), function->inputs().size()),
    });
    current_scope()->PopValue(); // function handle
    for (const auto& input : function->inputs()) {
      current_scope()->PopValue();
    }
    for (const auto& output : function->outputs()) {
      current_scope()->PushValue(output.type);
    }
  }
}

void ScriptFunctionParser::InsertInstructions(std::string_view path, std::initializer_list<Instruction> instructions) {
  definition.instructions.insert(definition.instructions.end(), instructions);
}

// std::vector<Function::ValueDeclaration> ScriptFunctionParser::ParseInputOutputDeclarations(
//     const json& value_declarations, std::string path) {
//   if (!value_declarations.is_array()) {
//     errors.push_back({
//         .message = "Value declarations must be an array.",
//         .path = path,
//     });
//     return {};
//   } else {
//     std::vector<Function::ValueDeclaration> declarations;
//     for (const auto& declaration : value_declarations) {
//       const auto parsed_declaration = ParseInputOutputDeclaration(declaration, path);
//       if (parsed_declaration.has_value()) {
//         declarations.push_back(std::move(*parsed_declaration));
//       }
//     }
//     return declarations;
//   }
// }

// std::optional<Function::ValueDeclaration> ScriptFunctionParser::ParseInputOutputDeclaration(
//     const json& value_declaration, std::string path) {
//   if (!value_declaration.is_object()) {
//     errors.push_back({
//         .message = "Value declaration must be an object.",
//         .path = path,
//     });
//     return {};
//   } else {
//     const std::string module = value_declaration["module"];
//     const std::string type = value_declaration["type"];
//     const std::string name = value_declaration["name"];

//     return Function::ValueDeclaration {
//       .name = name,
//       .type = Module::Get(module)->GetType(type),
//     };
//   }
// }

// void ScriptFunctionParser::ParseStatements(const json& statements, std::string path) {
//   if (!statements.is_array()) {
//     errors.push_back({
//         .message = "Statements must be an array.",
//         .path = path,
//     });
//   } else {
//     for (std::size_t i = 0, size = statements.size(); i < size; ++i) {
//       ParseStatement(statements[i], fmt::format("{}/{}", path, i));
//     }
//   }
// }

// void ScriptFunctionParser::ParseStatement(const json& statement, std::string path) {
//   if (!statement.is_object()) {
//     errors.push_back({
//         .message = "Statement must be an object.",
//         .path = path,
//     });
//   } else if (const auto type_it = statement.find("type"); type_it == statement.end()) {
//     errors.push_back({
//         .message = "Statement must contain key 'type'.",
//         .path = path,
//     });
//   } else if (!type_it->is_string()) {
//     errors.push_back({
//         .message = "Key 'type' must be a string.",
//         .path = path,
//     });
//   } else if (*type_it == "function_call") {
//     ParseFunctionCallStatement(statement, path);
//   } else if (*type_it == "if") {
//     ParseIf(statement, path);
//   } else if (*type_it == "while") {
//     ParseWhile(statement, path);
//   } else if (*type_it == "return") {
//     ParseReturn(statement, path);
//   } else {
//     errors.push_back({
//         .message = fmt::format("Unknown value for statement type: '{}'", *type_it),
//         .path = path,
//     });
//   }
// }

// void ScriptFunctionParser::ParseFunctionCallStatement(const json& statement, std::string path) {
//   assert(statement.is_object()); // Should have already been checked before
//   if (const auto function = statement.find("function"); function == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("Function call does not include key 'function'."),
//         .path = path,
//     });
//   } else if (!function->is_object()) {
//     errors.push_back({
//         .message = fmt::format("Function key must be an object."),
//         .path = path,
//     });
//   } else if (const auto function_name = function->find("name"); function_name == function->end()) {
//     errors.push_back({
//         .message = fmt::format("Function must contain key 'name'."),
//         .path = path,
//     });
//   } else if (!function_name->is_string()) {
//     errors.push_back({
//         .message = fmt::format("Function key 'module' must be of type string."),
//         .path = path,
//     });
//   } else if (const auto function_module = function->find("module"); function_module == function->end()) {
//     errors.push_back({
//         .message = fmt::format("Function must contain key 'module'."),
//         .path = path,
//     });
//   } else if (!function_module->is_string()) {
//     errors.push_back({
//         .message = fmt::format("Function key 'module' must be of type string."),
//         .path = path,
//     });
//   } else if (std::shared_ptr<Module> module = Module::Get(static_cast<std::string>(*function_module)); module == nullptr) {
//     errors.push_back({
//         .message = fmt::format("Module {} not found.", *function_module),
//         .path = path,
//     });
//   } else if (std::shared_ptr<Function> function_reflection = module->GetFunction(static_cast<std::string>(*function_name)); function_reflection == nullptr) {
//     errors.push_back({
//         .message = fmt::format("Function {} not found in module {}.", *function_name, *function_module),
//         .path = path,
//     });
//   } else if (const auto& outputs = statement.find("outputs"); outputs == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("Function call statement must contain key 'outputs'."),
//         .path = path,
//     });
//   } else if (!outputs->is_object()) {
//     errors.push_back({
//         .message = fmt::format("Function key 'outputs' must be an object."),
//         .path = path,
//     });
//   } else if (const auto& inputs = statement.find("inputs"); inputs == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("Function call statement must contain key 'inputs'."),
//         .path = path,
//     });
//   } else if (!inputs->is_object()) {
//     errors.push_back({
//         .message = fmt::format("Function key 'inputs' must be an object."),
//         .path = path,
//     });
//   } else {
//     std::vector<std::optional<std::size_t>> output_positions(function_reflection->outputs().size());

//     for (const auto& [output_name, local_variable] : outputs->items()) {
//       if (!local_variable.is_string()) {
//         errors.push_back({
//             .message = fmt::format("Local variable name of output '{}' must be a string.", output_name),
//             .path = path,
//         });
//       } else if (const auto& output_declaration = function_reflection->GetOutput(output_name); !output_declaration.has_value()) {
//         errors.push_back({
//             .message = fmt::format("Output '{}' not present for function.", output_name, function_reflection->name()),
//             .path = path,
//         });
//       } else if (const auto output_variable_position = GetOutputVariablePosition(
//                      static_cast<std::string>(local_variable), output_declaration->type.lock(), path);
//                  !output_variable_position.has_value()) {
//         errors.push_back({
//             .message = fmt::format("Mismatched types."),
//             .path = path,
//         });
//       } else {
//         assert(function_reflection->GetOutputIndex(output_name).has_value());
//         output_positions[*function_reflection->GetOutputIndex(output_name)] = *output_variable_position;
//       }
//     }

//     instructions.push_back(instructions::PushStackFrame {});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });

//     for (const auto& input_declaration : function_reflection->inputs()) {
//       if (const auto& input_definition = inputs->find(input_declaration.name); input_definition == inputs->end()) {
//         errors.push_back({
//             .message = fmt::format("Missing input: '{}'.", input_declaration.name),
//             .path = path,
//         });
//       } else {
//         ParsePush(*input_definition, path, input_declaration.type.lock(), 1);
//       }
//     }

//     instructions.push_back(instructions::NativeFunctionCall {
//         .function_pointer = function_reflection->pointer(),
//     });
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });

//     // TODO: Use ranges::reverse
//     for (auto it = output_positions.rbegin(); it != output_positions.rend(); ++it) {
//       const auto& output_position = *it;
//       if (output_position.has_value()) {
//         instructions.push_back(instructions::Assign {
//             .position = *output_position,
//             .stack_frame_offset = 1,
//         });
//         debug_info.instruction_info.push_back({
//           .scope = current_scope_index,
//           .statement = path,
//         });
//       } else {
//         instructions.push_back(instructions::Pop {
//             .count = 1,
//         });
//         debug_info.instruction_info.push_back({
//           .scope = current_scope_index,
//           .statement = path,
//         });
//       }
//     }
//     instructions.push_back(instructions::PopStackFrame {});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });
//   }
// }

// void ScriptFunctionParser::ParseIf(const json& statement, std::string path) {
//   assert(statement.is_object()); // Should have already been checked before
//   if (const auto condition = statement.find("condition"); condition == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'condition'."),
//         .path = path,
//     });
//   } else if (const auto statements = statement.find("statements"); statements == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'statements'."),
//         .path = path,
//     });
//   } else {
//     ParsePush(*condition, path, virtual_machine->GetType<bool>());

//     const std::size_t instruction_count_before = instructions.size();
//     instructions.push_back(instructions::JumpIfFalse{});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });
//     ParseStatements(*statements, fmt::format("{}/statements", path));
//     const std::size_t instruction_count_after = instructions.size();

//     assert(instruction_count_after >= instruction_count_before);
//     std::get<instructions::JumpIfFalse>(instructions[instruction_count_before]).instruction_offset =
//         instruction_count_after - instruction_count_before;
//   }
// }

// void ScriptFunctionParser::ParseWhile(const json& statement, std::string path) {
//   assert(statement.is_object()); // Should have already been checked before
//   if (const auto condition = statement.find("condition"); condition == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'condition'."),
//         .path = path,
//     });
//   } else if (const auto statements = statement.find("statements"); statements == statement.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'statements'."),
//         .path = path,
//     });
//   } else {
//     const std::ptrdiff_t instruction_count_before_push = instructions.size();
//     ParsePush(*condition, path, virtual_machine->GetType<bool>());
//     const std::ptrdiff_t instruction_count_after_push = instructions.size();

//     instructions.push_back(instructions::JumpIfFalse{});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });
//     ParseStatements(*statements, fmt::format("{}/statements", path));
//     instructions.push_back(instructions::Jump {
//         .instruction_offset = instruction_count_before_push - static_cast<std::ptrdiff_t>(instructions.size()),
//     });
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });

//     std::get<instructions::JumpIfFalse>(instructions[instruction_count_after_push]).instruction_offset =
//         instructions.size() - instruction_count_after_push;
//   }
// }
// void ScriptFunctionParser::ParseReturn(const json& statement, std::string path) {
//   assert(statement.is_object()); // Should have already been checked before
//   if (const auto returned_outputs = statement.find("outputs"); returned_outputs == statement.end()) {
//     if (outputs.size() == 0) {
//     } else {
//       errors.push_back({
//           .message = fmt::format("Return requires key 'outputs' if function has at least one output."),
//           .path = path,
//       });
//     }
//   } else {
//     for (const auto& output : this->outputs) {
//       const auto& returned_output = returned_outputs->find(output.name);
//       if (returned_output == returned_outputs->end()) {
//         errors.push_back({
//             .message = fmt::format("Missing return value for '{}'", output.name),
//             .path = path,
//         });
//         PushNone(path);
//       } else {
//         ParsePush(*returned_output, path, output.type.lock());
//       }
//     }
//     instructions.push_back(instructions::Return {});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//     });
//   }
// }

// void ScriptFunctionParser::PushNone(const std::string& path) {
//   instructions.push_back(instructions::PushConstant {
//       .value = Value::None()
//   });
//   debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .statement = path,
//   });
// }

// void ScriptFunctionParser::ParsePush(const json& value_definition, const std::string& path, std::shared_ptr<Type> required_type, std::size_t stack_frame_offset) {
//   auto push = [&](auto value) {
//     const auto input_type = virtual_machine->GetType<decltype(value)>();
//     if (!required_type || required_type == input_type) {
//       instructions.push_back(instructions::PushConstant{
//           .value = Value::Create(value),
//       });
//       debug_info.instruction_info.push_back({
//           .scope = current_scope_index,
//           .statement = path,
//       });
//     } else {
//       errors.push_back({
//           .message = fmt::format("Invalid input: expected '{}' got '{}'.", required_type->name(),
//                                  input_type ? input_type->name() : "Unknown"),
//           .path = path,
//       });
//     }
//   };

//   if (value_definition.is_string()) {
//     push(static_cast<std::string>(value_definition));
//   } else if (value_definition.is_number()) {
//     push(static_cast<double>(value_definition));
//   } else if (value_definition.is_boolean()) {
//     push(static_cast<bool>(value_definition));
//   } else if (value_definition.is_object() && value_definition.contains("inputType")) {
//     const std::string& input_type = value_definition["inputType"];

//     if (input_type == "local_variable" && value_definition.contains("name")) {
//       const std::string& local_variable_name = value_definition["name"];
//       const auto local_variable = GetLocalVariable(local_variable_name);
//       if (!local_variable.has_value()) {
//         errors.push_back({
//             .message = fmt::format("Cannot find local variable '{}'.", local_variable_name),
//             .path = path,
//         });
//       } else {
//         instructions.push_back(instructions::PushStackValue{
//             .position = local_variable->position,
//             .stack_frame_offset = stack_frame_offset,
//         });
//         debug_info.instruction_info.push_back({
//             .scope = current_scope_index,
//             .statement = path,
//         });
//       }
//     } else if (input_type == "constant" && value_definition.contains("type") && value_definition.contains("value")) {
//       const std::string& type = value_definition["type"];
//       const json& value = value_definition["value"];
//       if (type == "Text" && value.is_string()) {
//         push(static_cast<std::string>(value));
//       } else if (type == "Number" && value.is_number()) {
//         push(static_cast<double>(value));
//       } else if (type == "Boolean" && value.is_boolean()) {
//         push(static_cast<bool>(value));
//       } else {
//         errors.push_back({
//             .message = fmt::format("Invalid constant type '{}' for value '{}'.", type, value.dump()),
//             .path = path,
//         });
//       }
//     } else {
//       errors.push_back({
//           .message = fmt::format("Invalid input definition."),
//           .path = path,
//       });
//     }
//   } else {
//     errors.push_back({
//         .message = fmt::format("Invalid input definition."),
//         .path = path,
//     });
//   }
// }

// std::optional<std::size_t> ScriptFunctionParser::GetOutputVariablePosition(std::string_view name, std::shared_ptr<Type> type, const std::string& path) {
//   const auto local_variable = GetLocalVariable(name);
//   if (local_variable.has_value()) {
//     if (local_variable->declaration.type.lock() == type) {
//       return local_variable->position;
//     } else {
//       return {};
//     }
//   } else {
//     PushNone(path);
//     const std::size_t position = current_scope().position_offset + current_scope().variables.size();
//     current_scope().variables.push_back({
//         .declaration = {
//           .name = std::string(name),
//           .type = type,
//         },
//         .position = position,
//     });
//     return position;
//   }
// }

// std::optional<ScriptFunction::DebugInfo::ScriptFunctionScope::Variable> ScriptFunctionParser::GetLocalVariable(std::string_view name) {
//   std::size_t scope_index = current_scope_index;
//   do {
//     const ScriptFunction::DebugInfo::ScriptFunctionScope& scope = debug_info.scope_info[scope_index];
//     for (const auto& variable : scope.variables) {
//       if (variable.declaration.name == name) {
//         return variable;
//       }
//     }
//     scope_index = scope.parent_scope;
//   } while (scope_index != std::numeric_limits<std::size_t>::max());

//   return {};
// }

// void ScriptFunctionParser::PushScope() {
//   debug_info.scope_info.push_back({
//       .parent_scope = current_scope_index,
//       .variables = {},
//       .position_offset = current_scope_index == std::numeric_limits<std::size_t>::max() ? 0 : static_cast<int>(current_scope().position_offset + current_scope().variables.size())
//   });
//   current_scope_index = debug_info.scope_info.size() - 1;
// }

// void ScriptFunctionParser::PopScope() {
//   assert(debug_info.scope_info.size() > 0);
//   current_scope_index = current_scope().parent_scope;
// }

}
