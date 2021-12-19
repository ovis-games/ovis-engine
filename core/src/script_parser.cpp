#include <ovis/core/script_parser.hpp>
#include <ovis/core/script_function.hpp>

namespace ovis {

namespace {

struct ScopeValue {
  Type::Id type_id;
  std::optional<std::string> variable_name;
};

struct Scope {
  Scope* parent = nullptr;
  std::vector<ScopeValue> values;
};

struct ScriptFunctionParser {
  std::vector<Scope> scopes;
  ParseScriptFunctionResult result;
  ParseScriptErrors errors;

  void Parse(const json& action_definiton);
  void ParseActions(const json& action_definiton, std::string_view path);
  void ParseAction(const json& action_definiton, std::string_view path);
  void ParseFunctionCall(const json& action_definiton, std::string_view path);
  void ParsePushValue(const json& value_definition, std::string_view path, Type::Id type);
  void ParsePushVariable(const json& value_definition, std::string_view path, Type::Id type);
  void ParsePushVariableReference(const json& value_definition, std::string_view path, Type::Id type);
};

}  // namespace

Result<ParseScriptFunctionResult, ParseScriptErrors> ParseScriptFunction(const json& function_definition) {
  ScriptFunctionParser parser;
  parser.Parse(function_definition);
  if (parser.errors.size() > 0) {
    return parser.errors;
  } else {
    return parser.result;
  }
}

namespace {

void ScriptFunctionParser::Parse(const json& json_definition) {
  // TODO: check format
  scopes.push_back({});
  ParseActions(json_definition["actions"], "/actions");
}

void ScriptFunctionParser::ParseActions(const json& actions_definiton, std::string_view path) {
  for (const auto& [index, action_definition] : actions_definiton.items()) {
    assert(std::stoi(index) >= 0);
    ParseAction(action_definition, fmt::format("{}/{}", path, index));
  }
}

void ScriptFunctionParser::ParseAction(const json& action_definiton, std::string_view path) {
  const std::string& id = action_definiton["id"];
  if (id == "function_call") {
    ParseFunctionCall(action_definiton, path);
  } else {
    errors.emplace_back(fmt::format("Invalid action id: {}", id), path);
  }
}

void ScriptFunctionParser::ParseFunctionCall(const json& action_definiton, std::string_view path) {
  assert(action_definiton["id"] == "function_call");
  const auto function = Function::Deserialize(action_definiton["function"]);
  if (!function) {
    errors.emplace_back(fmt::format("Unknown function: {}", action_definiton["function"].dump(), path));
    return;
  }
  
  for (const auto& input : function->inputs()) {
  }
}

void ScriptFunctionParser::ParsePushValue(const json& value_definition, std::string_view path, Type::Id type) {
  if (value_definition.is_string()) {
    if (type == Type::GetId<std::string>()) {
      errors.emplace_back("Parsing constant Core.String not implemented yet", path);
    } else {
      const auto requested_type = Type::Get(type);
      errors.emplace_back(fmt::format("Expected {}, got {}", requested_type ? requested_type->full_reference() : "Unknown", "Core.String"));
    }
  } else if (value_definition.is_number()) {
    if (type == Type::GetId<double>()) {
      errors.emplace_back("Parsing constant Core.Number not implemented yet", path);
    } else {
      const auto requested_type = Type::Get(type);
      errors.emplace_back(fmt::format("Expected {}, got {}", requested_type ? requested_type->full_reference() : "Unknown", "Core.Number"));
    }
  } else if (value_definition.is_boolean()) {
    if (type == Type::GetId<bool>()) {
      errors.emplace_back("Parsing constant Core.Number not implemented yet", path);
    } else {
      const auto requested_type = Type::Get(type);
      errors.emplace_back(fmt::format("Expected {}, got {}", requested_type ? requested_type->full_reference() : "Unknown", "Core.Boolean"));
    }
  } else if (value_definition.is_object()) {
    const std::string& id = value_definition["id"];
    if (id == "variable") {
      ParsePushVariable(value_definition, path, type);
    } else {
      errors.emplace_back("Invalid value definition", path);
    }
  } else {
    errors.emplace_back("Invalid value definition", path);
  }
}

void ScriptFunctionParser::ParsePushVariable(const json& value_definition, std::string_view path, Type::Id type) {
  assert(value_definition["id"] == "variable");
}

}  // namespace


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

// void ScriptFunctionParser::ParseActions(const json& actions, std::string path) {
//   if (!actions.is_array()) {
//     errors.push_back({
//         .message = "Actions must be an array.",
//         .path = path,
//     });
//   } else {
//     for (std::size_t i = 0, size = actions.size(); i < size; ++i) {
//       ParseAction(actions[i], fmt::format("{}/{}", path, i));
//     }
//   }
// }

// void ScriptFunctionParser::ParseAction(const json& action, std::string path) {
//   if (!action.is_object()) {
//     errors.push_back({
//         .message = "Action must be an object.",
//         .path = path,
//     });
//   } else if (const auto type_it = action.find("type"); type_it == action.end()) {
//     errors.push_back({
//         .message = "Action must contain key 'type'.",
//         .path = path,
//     });
//   } else if (!type_it->is_string()) {
//     errors.push_back({
//         .message = "Key 'type' must be a string.",
//         .path = path,
//     });
//   } else if (*type_it == "function_call") {
//     ParseFunctionCallAction(action, path);
//   } else if (*type_it == "if") {
//     ParseIf(action, path);
//   } else if (*type_it == "while") {
//     ParseWhile(action, path);
//   } else if (*type_it == "return") {
//     ParseReturn(action, path);
//   } else {
//     errors.push_back({
//         .message = fmt::format("Unknown value for action type: '{}'", *type_it),
//         .path = path,
//     });
//   }
// }

// void ScriptFunctionParser::ParseFunctionCallAction(const json& action, std::string path) {
//   assert(action.is_object()); // Should have already been checked before
//   if (const auto function = action.find("function"); function == action.end()) {
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
//   } else if (const auto& outputs = action.find("outputs"); outputs == action.end()) {
//     errors.push_back({
//         .message = fmt::format("Function call action must contain key 'outputs'."),
//         .path = path,
//     });
//   } else if (!outputs->is_object()) {
//     errors.push_back({
//         .message = fmt::format("Function key 'outputs' must be an object."),
//         .path = path,
//     });
//   } else if (const auto& inputs = action.find("inputs"); inputs == action.end()) {
//     errors.push_back({
//         .message = fmt::format("Function call action must contain key 'inputs'."),
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
//       .action = path,
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
//       .action = path,
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
//           .action = path,
//         });
//       } else {
//         instructions.push_back(instructions::Pop {
//             .count = 1,
//         });
//         debug_info.instruction_info.push_back({
//           .scope = current_scope_index,
//           .action = path,
//         });
//       }
//     }
//     instructions.push_back(instructions::PopStackFrame {});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .action = path,
//     });
//   }
// }

// void ScriptFunctionParser::ParseIf(const json& action, std::string path) {
//   assert(action.is_object()); // Should have already been checked before
//   if (const auto condition = action.find("condition"); condition == action.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'condition'."),
//         .path = path,
//     });
//   } else if (const auto actions = action.find("actions"); actions == action.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'actions'."),
//         .path = path,
//     });
//   } else {
//     ParsePush(*condition, path, Type::Get<bool>());

//     const std::size_t instruction_count_before = instructions.size();
//     instructions.push_back(instructions::JumpIfFalse{});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .action = path,
//     });
//     ParseActions(*actions, fmt::format("{}/actions", path));
//     const std::size_t instruction_count_after = instructions.size();

//     assert(instruction_count_after >= instruction_count_before);
//     std::get<instructions::JumpIfFalse>(instructions[instruction_count_before]).instruction_offset =
//         instruction_count_after - instruction_count_before;
//   }
// }

// void ScriptFunctionParser::ParseWhile(const json& action, std::string path) {
//   assert(action.is_object()); // Should have already been checked before
//   if (const auto condition = action.find("condition"); condition == action.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'condition'."),
//         .path = path,
//     });
//   } else if (const auto actions = action.find("actions"); actions == action.end()) {
//     errors.push_back({
//         .message = fmt::format("If requires key 'actions'."),
//         .path = path,
//     });
//   } else {
//     const std::ptrdiff_t instruction_count_before_push = instructions.size();
//     ParsePush(*condition, path, Type::Get<bool>());
//     const std::ptrdiff_t instruction_count_after_push = instructions.size();

//     instructions.push_back(instructions::JumpIfFalse{});
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .action = path,
//     });
//     ParseActions(*actions, fmt::format("{}/actions", path));
//     instructions.push_back(instructions::Jump {
//         .instruction_offset = instruction_count_before_push - static_cast<std::ptrdiff_t>(instructions.size()),
//     });
//     debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .action = path,
//     });

//     std::get<instructions::JumpIfFalse>(instructions[instruction_count_after_push]).instruction_offset =
//         instructions.size() - instruction_count_after_push;
//   }
// }
// void ScriptFunctionParser::ParseReturn(const json& action, std::string path) {
//   assert(action.is_object()); // Should have already been checked before
//   if (const auto returned_outputs = action.find("outputs"); returned_outputs == action.end()) {
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
//       .action = path,
//     });
//   }
// }

// void ScriptFunctionParser::PushNone(const std::string& path) {
//   instructions.push_back(instructions::PushConstant {
//       .value = Value::None()
//   });
//   debug_info.instruction_info.push_back({
//       .scope = current_scope_index,
//       .action = path,
//   });
// }

// void ScriptFunctionParser::ParsePush(const json& value_definition, const std::string& path, std::shared_ptr<Type> required_type, std::size_t stack_frame_offset) {
//   auto push = [&](auto value) {
//     const auto input_type = Type::Get<decltype(value)>();
//     if (!required_type || required_type == input_type) {
//       instructions.push_back(instructions::PushConstant{
//           .value = Value::Create(value),
//       });
//       debug_info.instruction_info.push_back({
//           .scope = current_scope_index,
//           .action = path,
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
//             .action = path,
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

// std::optional<ScriptFunction::DebugInfo::Scope::Variable> ScriptFunctionParser::GetLocalVariable(std::string_view name) {
//   std::size_t scope_index = current_scope_index;
//   do {
//     const ScriptFunction::DebugInfo::Scope& scope = debug_info.scope_info[scope_index];
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
