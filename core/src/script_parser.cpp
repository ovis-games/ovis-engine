#include <ovis/core/script_parser.hpp>
#include <ovis/core/script_function.hpp>

namespace ovis {

ScriptFunctionParser::ScriptFunctionParser(const json& function_definition) {
  if (!function_definition.is_object()) {
    errors.push_back({
        .message = "Function definition must be an object",
        .path = "/",
    });
  } else {
    PushScope();
    inputs = ParseInputOutputDeclarations(function_definition["inputs"], "/inputs");
    for (const auto& input : IndexRange(inputs)) {
      debug_info.scope_info[0].variables.push_back({
          .declaration = *input,
          .position = input.index(),
      });
    }
    outputs = ParseInputOutputDeclarations(function_definition["outputs"], "/outputs");
    ParseActions(function_definition["actions"], "/actions");
    PopScope();
  }
}

std::vector<vm::Function::ValueDeclaration> ScriptFunctionParser::ParseInputOutputDeclarations(
    const json& value_declarations, std::string path) {
  if (!value_declarations.is_array()) {
    errors.push_back({
        .message = "Value declarations must be an array.",
        .path = path,
    });
    return {};
  } else {
    std::vector<vm::Function::ValueDeclaration> declarations;
    for (const auto& declaration : value_declarations) {
      const auto parsed_declaration = ParseInputOutputDeclaration(declaration, path);
      if (parsed_declaration.has_value()) {
        declarations.push_back(std::move(*parsed_declaration));
      }
    }
    return declarations;
  }
}

std::optional<vm::Function::ValueDeclaration> ScriptFunctionParser::ParseInputOutputDeclaration(
    const json& value_declaration, std::string path) {
  if (!value_declaration.is_object()) {
    errors.push_back({
        .message = "Value declaration must be an object.",
        .path = path,
    });
    return {};
  } else {
    const std::string module = value_declaration["module"];
    const std::string type = value_declaration["type"];
    const std::string name = value_declaration["name"];

    return vm::Function::ValueDeclaration {
      .name = name,
      .type = vm::Module::Get(module)->GetType(type),
    };
  }
}

void ScriptFunctionParser::ParseActions(const json& actions, std::string path) {
  if (!actions.is_array()) {
    errors.push_back({
        .message = "Actions must be an array.",
        .path = path,
    });
  } else {
    for (std::size_t i = 0, size = actions.size(); i < size; ++i) {
      ParseAction(actions[i], fmt::format("{}/{}", path, i));
    }
  }
}

void ScriptFunctionParser::ParseAction(const json& action, std::string path) {
  if (!action.is_object()) {
    errors.push_back({
        .message = "Action must be an object.",
        .path = path,
    });
  } else if (const auto type_it = action.find("type"); type_it == action.end()) {
    errors.push_back({
        .message = "Action must contain key 'type'.",
        .path = path,
    });
  } else if (!type_it->is_string()) {
    errors.push_back({
        .message = "Key 'type' must be a string.",
        .path = path,
    });
  } else if (*type_it == "function_call") {
    ParseFunctionCallAction(action, path);
  } else if (*type_it == "if") {
    ParseIf(action, path);
  } else if (*type_it == "while") {
    ParseWhile(action, path);
  } else if (*type_it == "return") {
    ParseReturn(action, path);
  } else {
    errors.push_back({
        .message = fmt::format("Unknown value for action type: '{}'", *type_it),
        .path = path,
    });
  }
}

void ScriptFunctionParser::ParseFunctionCallAction(const json& action, std::string path) {
  assert(action.is_object()); // Should have already been checked before
  if (const auto function = action.find("function"); function == action.end()) {
    errors.push_back({
        .message = fmt::format("Function call does not include key 'function'."),
        .path = path,
    });
  } else if (!function->is_object()) {
    errors.push_back({
        .message = fmt::format("Function key must be an object."),
        .path = path,
    });
  } else if (const auto function_name = function->find("name"); function_name == function->end()) {
    errors.push_back({
        .message = fmt::format("Function must contain key 'name'."),
        .path = path,
    });
  } else if (!function_name->is_string()) {
    errors.push_back({
        .message = fmt::format("Function key 'module' must be of type string."),
        .path = path,
    });
  } else if (const auto function_module = function->find("module"); function_module == function->end()) {
    errors.push_back({
        .message = fmt::format("Function must contain key 'module'."),
        .path = path,
    });
  } else if (!function_module->is_string()) {
    errors.push_back({
        .message = fmt::format("Function key 'module' must be of type string."),
        .path = path,
    });
  } else if (std::shared_ptr<vm::Module> module = vm::Module::Get(static_cast<std::string>(*function_module)); module == nullptr) {
    errors.push_back({
        .message = fmt::format("Module {} not found.", *function_module),
        .path = path,
    });
  } else if (std::shared_ptr<vm::Function> function_reflection = module->GetFunction(static_cast<std::string>(*function_name)); function_reflection == nullptr) {
    errors.push_back({
        .message = fmt::format("Function {} not found in module {}.", *function_name, *function_module),
        .path = path,
    });
  } else if (const auto& outputs = action.find("outputs"); outputs == action.end()) {
    errors.push_back({
        .message = fmt::format("Function call action must contain key 'outputs'."),
        .path = path,
    });
  } else if (!outputs->is_object()) {
    errors.push_back({
        .message = fmt::format("Function key 'outputs' must be an object."),
        .path = path,
    });
  } else if (const auto& inputs = action.find("inputs"); inputs == action.end()) {
    errors.push_back({
        .message = fmt::format("Function call action must contain key 'inputs'."),
        .path = path,
    });
  } else if (!inputs->is_object()) {
    errors.push_back({
        .message = fmt::format("Function key 'inputs' must be an object."),
        .path = path,
    });
  } else {
    std::vector<std::optional<std::size_t>> output_positions(function_reflection->outputs().size());

    for (const auto& [output_name, local_variable] : outputs->items()) {
      if (!local_variable.is_string()) {
        errors.push_back({
            .message = fmt::format("Local variable name of output '{}' must be a string.", output_name),
            .path = path,
        });
      } else if (const auto& output_declaration = function_reflection->GetOutput(output_name); !output_declaration.has_value()) {
        errors.push_back({
            .message = fmt::format("Output '{}' not present for function.", output_name, function_reflection->name()),
            .path = path,
        });
      } else if (const auto output_variable_position = GetOutputVariablePosition(
                     static_cast<std::string>(local_variable), output_declaration->type.lock(), path);
                 !output_variable_position.has_value()) {
        errors.push_back({
            .message = fmt::format("Mismatched types."),
            .path = path,
        });
      } else {
        assert(function_reflection->GetOutputIndex(output_name).has_value());
        output_positions[*function_reflection->GetOutputIndex(output_name)] = *output_variable_position;
      }
    }

    instructions.push_back(vm::instructions::PushStackFrame {});
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });

    for (const auto& input_declaration : function_reflection->inputs()) {
      if (const auto& input_definition = inputs->find(input_declaration.name); input_definition == inputs->end()) {
        errors.push_back({
            .message = fmt::format("Missing input: '{}'.", input_declaration.name),
            .path = path,
        });
      } else {
        ParsePush(*input_definition, path, input_declaration.type.lock(), 1);
      }
    }

    instructions.push_back(vm::instructions::NativeFunctionCall {
        .function_pointer = function_reflection->pointer(),
    });
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });

    // TODO: Use ranges::reverse
    for (auto it = output_positions.rbegin(); it != output_positions.rend(); ++it) {
      const auto& output_position = *it;
      if (output_position.has_value()) {
        instructions.push_back(vm::instructions::Assign {
            .position = *output_position,
            .stack_frame_offset = 1,
        });
        debug_info.instruction_info.push_back({
          .scope = current_scope_index,
          .action = path,
        });
      } else {
        instructions.push_back(vm::instructions::Pop {
            .count = 1,
        });
        debug_info.instruction_info.push_back({
          .scope = current_scope_index,
          .action = path,
        });
      }
    }
    instructions.push_back(vm::instructions::PopStackFrame {});
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });
  }
}

void ScriptFunctionParser::ParseIf(const json& action, std::string path) {
  assert(action.is_object()); // Should have already been checked before
  if (const auto condition = action.find("condition"); condition == action.end()) {
    errors.push_back({
        .message = fmt::format("If requires key 'condition'."),
        .path = path,
    });
  } else if (const auto actions = action.find("actions"); actions == action.end()) {
    errors.push_back({
        .message = fmt::format("If requires key 'actions'."),
        .path = path,
    });
  } else {
    ParsePush(*condition, path, vm::Type::Get<bool>());

    const std::size_t instruction_count_before = instructions.size();
    instructions.push_back(vm::instructions::JumpIfFalse{});
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });
    ParseActions(*actions, fmt::format("{}/actions", path));
    const std::size_t instruction_count_after = instructions.size();

    assert(instruction_count_after >= instruction_count_before);
    std::get<vm::instructions::JumpIfFalse>(instructions[instruction_count_before]).instruction_offset =
        instruction_count_after - instruction_count_before;
  }
}

void ScriptFunctionParser::ParseWhile(const json& action, std::string path) {
  assert(action.is_object()); // Should have already been checked before
  if (const auto condition = action.find("condition"); condition == action.end()) {
    errors.push_back({
        .message = fmt::format("If requires key 'condition'."),
        .path = path,
    });
  } else if (const auto actions = action.find("actions"); actions == action.end()) {
    errors.push_back({
        .message = fmt::format("If requires key 'actions'."),
        .path = path,
    });
  } else {
    const std::ptrdiff_t instruction_count_before_push = instructions.size();
    ParsePush(*condition, path, vm::Type::Get<bool>());
    const std::ptrdiff_t instruction_count_after_push = instructions.size();

    instructions.push_back(vm::instructions::JumpIfFalse{});
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });
    ParseActions(*actions, fmt::format("{}/actions", path));
    instructions.push_back(vm::instructions::Jump {
        .instruction_offset = instruction_count_before_push - static_cast<std::ptrdiff_t>(instructions.size()),
    });
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });

    std::get<vm::instructions::JumpIfFalse>(instructions[instruction_count_after_push]).instruction_offset =
        instructions.size() - instruction_count_after_push;
  }
}
void ScriptFunctionParser::ParseReturn(const json& action, std::string path) {
  assert(action.is_object()); // Should have already been checked before
  if (const auto returned_outputs = action.find("outputs"); returned_outputs == action.end()) {
    if (outputs.size() == 0) {
    } else {
      errors.push_back({
          .message = fmt::format("Return requires key 'outputs' if function has at least one output."),
          .path = path,
      });
    }
  } else {
    for (const auto& output : this->outputs) {
      const auto& returned_output = returned_outputs->find(output.name);
      if (returned_output == returned_outputs->end()) {
        errors.push_back({
            .message = fmt::format("Missing return value for '{}'", output.name),
            .path = path,
        });
        PushNone(path);
      } else {
        ParsePush(*returned_output, path, output.type.lock());
      }
    }
    instructions.push_back(vm::instructions::Return {});
    debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
    });
  }
}

void ScriptFunctionParser::PushNone(const std::string& path) {
  instructions.push_back(vm::instructions::PushConstant {
      .value = vm::Value::None()
  });
  debug_info.instruction_info.push_back({
      .scope = current_scope_index,
      .action = path,
  });
}

void ScriptFunctionParser::ParsePush(const json& value_definition, const std::string& path, std::shared_ptr<vm::Type> required_type, std::size_t stack_frame_offset) {
  auto push = [&](auto value) {
    const auto input_type = vm::Type::Get<decltype(value)>();
    if (!required_type || required_type == input_type) {
      instructions.push_back(vm::instructions::PushConstant{
          .value = vm::Value::Create(value),
      });
      debug_info.instruction_info.push_back({
          .scope = current_scope_index,
          .action = path,
      });
    } else {
      errors.push_back({
          .message = fmt::format("Invalid input: expected '{}' got '{}'.", required_type->name(),
                                 input_type ? input_type->name() : "Unknown"),
          .path = path,
      });
    }
  };

  if (value_definition.is_string()) {
    push(static_cast<std::string>(value_definition));
  } else if (value_definition.is_number()) {
    push(static_cast<double>(value_definition));
  } else if (value_definition.is_boolean()) {
    push(static_cast<bool>(value_definition));
  } else if (value_definition.is_object() && value_definition.contains("inputType")) {
    const std::string& input_type = value_definition["inputType"];

    if (input_type == "local_variable" && value_definition.contains("name")) {
      const std::string& local_variable_name = value_definition["name"];
      const auto local_variable = GetLocalVariable(local_variable_name);
      if (!local_variable.has_value()) {
        errors.push_back({
            .message = fmt::format("Cannot find local variable '{}'.", local_variable_name),
            .path = path,
        });
      } else {
        instructions.push_back(vm::instructions::PushStackValue{
            .position = local_variable->position,
            .stack_frame_offset = stack_frame_offset,
        });
        debug_info.instruction_info.push_back({
            .scope = current_scope_index,
            .action = path,
        });
      }
    } else if (input_type == "constant" && value_definition.contains("type") && value_definition.contains("value")) {
      const std::string& type = value_definition["type"];
      const json& value = value_definition["value"];
      if (type == "Text" && value.is_string()) {
        push(static_cast<std::string>(value));
      } else if (type == "Number" && value.is_number()) {
        push(static_cast<double>(value));
      } else if (type == "Boolean" && value.is_boolean()) {
        push(static_cast<bool>(value));
      } else {
        errors.push_back({
            .message = fmt::format("Invalid constant type '{}' for value '{}'.", type, value.dump()),
            .path = path,
        });
      }
    } else {
      errors.push_back({
          .message = fmt::format("Invalid input definition."),
          .path = path,
      });
    }
  } else {
    errors.push_back({
        .message = fmt::format("Invalid input definition."),
        .path = path,
    });
  }
}

std::optional<std::size_t> ScriptFunctionParser::GetOutputVariablePosition(std::string_view name, std::shared_ptr<vm::Type> type, const std::string& path) {
  const auto local_variable = GetLocalVariable(name);
  if (local_variable.has_value()) {
    if (local_variable->declaration.type.lock() == type) {
      return local_variable->position;
    } else {
      return {};
    }
  } else {
    PushNone(path);
    const std::size_t position = current_scope().position_offset + current_scope().variables.size();
    current_scope().variables.push_back({
        .declaration = {
          .name = std::string(name),
          .type = type,
        },
        .position = position,
    });
    return position;
  }
}

std::optional<ScriptFunction::DebugInfo::Scope::Variable> ScriptFunctionParser::GetLocalVariable(std::string_view name) {
  std::size_t scope_index = current_scope_index;
  do {
    const ScriptFunction::DebugInfo::Scope& scope = debug_info.scope_info[scope_index];
    for (const auto& variable : scope.variables) {
      if (variable.declaration.name == name) {
        return variable;
      }
    }
    scope_index = scope.parent_scope;
  } while (scope_index != std::numeric_limits<std::size_t>::max());

  return {};
}

void ScriptFunctionParser::PushScope() {
  debug_info.scope_info.push_back({
      .parent_scope = current_scope_index,
      .variables = {},
      .position_offset = current_scope_index == std::numeric_limits<std::size_t>::max() ? 0 : static_cast<int>(current_scope().position_offset + current_scope().variables.size())
  });
  current_scope_index = debug_info.scope_info.size() - 1;
}

void ScriptFunctionParser::PopScope() {
  assert(debug_info.scope_info.size() > 0);
  current_scope_index = current_scope().parent_scope;
}

}
