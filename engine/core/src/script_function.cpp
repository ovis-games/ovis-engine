#include "ovis/core/script_instruction.hpp"
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_execution_context.hpp>

namespace ovis {

namespace {

class ScriptFunctionParser {
 public:
  ScriptFunctionParser(const json& function_definition) {
    if (!function_definition.is_object()) {
      errors.push_back({
          .message = "Function definition must be an object",
          .path = "/",
      });
    } else {
      inputs = ParseInputOutputDeclarations(function_definition["inputs"]);
      outputs = ParseInputOutputDeclarations(function_definition["outputs"]);
      ParseActions(function_definition["actions"], "/actions");
    }
  }

 private:
  std::vector<Function::ValueDeclaration> ParseInputOutputDeclarations(const json& value_declarations) { return {}; }

  void ParseActions(const json& actions, std::string path) {
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

  void ParseAction(const json& action, std::string path) {
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
    } else {
      errors.push_back({
          .message = fmt::format("Unknown value for action type: '{}'", *type_it),
          .path = path,
      });
    }
  }

  void ParseFunctionCallAction(const json& action, std::string path) {
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
    } else if (safe_ptr<Module> module = Module::Get(static_cast<std::string>(*function_module)); module == nullptr) {
      errors.push_back({
          .message = fmt::format("Module {} not found.", *function_module),
          .path = path,
      });
    } else if (safe_ptr<Function> function = module->GetFunction(static_cast<std::string>(*function_name)); function != nullptr) {
      errors.push_back({
          .message = fmt::format("Function {} not found in module {}.", *function_name, *function_module),
          .path = path,
      });
    } else {
      // ...
      // const auto& function_identifier = static_cast<std::string>(action["function"]);
      // auto function = context_->GetFunction(function_identifier);
      // if (!function) {
      //   return ScriptError{.action = action_reference,
      //                      .message = fmt::format("Unknown function identifier '{}'", function_identifier)};
      // }

      // for (const auto& output : function->outputs) {
      //   if (action.contains(json::json_pointer(fmt::format("/outputs/{}", output.identifier)))) {
      //     const std::string variable_name = action["outputs"][output.identifier];
      //     const auto& existing_variable = GetLocalVariable(variable_name);

      //     if (!existing_variable.has_value()) {
      //       scope.instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
      //       scope.instruction_to_action_mappings.push_back(action_reference);
      //       local_variables_.push_back(LocalVariable{.name = variable_name,
      //                                                .declaring_action = action_reference,
      //                                                .type = output.type,
      //                                                .position = static_cast<int>(local_variables_.size())});
      //     } else if (existing_variable->type != output.type) {
      //       return ScriptError{
      //           .action = action_reference,
      //           .message = fmt::format("Assigning function output to local variable with different type. Output '{}'
      //           "
      //                                  "has type '{}', local variable '{}' has type '{}'",
      //                                  output.identifier, context_->GetType(output.type)->name, variable_name,
      //                                  context_->GetType(existing_variable->type)->name)};
      //     }
      //   }
      // }
      // // for (const auto& output : function->outputs) {
      // //   scope.instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
      // //   scope.instruction_to_action_mappings.push_back(action_reference);
      // // }
      // scope.instructions.push_back(Instruction{ .type = InstructionType::PUSH_STACK_FRAME });
      // scope.instruction_to_action_mappings.push_back(action_reference);

      // for (const auto& input : function->inputs) {
      //   if (!action.contains(json::json_pointer(fmt::format("/inputs/{}", input.identifier))) ||
      //       !PushValue(action["inputs"][input.identifier], &scope, -1)) {
      //     return ScriptError{.action = action_reference,
      //                        .message = fmt::format("No value for input '{}' provided", input.identifier)};
      //   }
      //   scope.instruction_to_action_mappings.push_back(action_reference);
      // }

      // // TODO: check input and output count before cast
      // if (const auto native_function = dynamic_cast<const NativeScriptFunction*>(function); native_function !=
      // nullptr) {
      //   scope.instructions.push_back(
      //       Instruction{InstructionType::FUNCTION_CALL,
      //                   FunctionCall{static_cast<uint8_t>(function->inputs.size()),
      //                                static_cast<uint8_t>(function->outputs.size()), native_function->function}});
      //   scope.instruction_to_action_mappings.push_back(action_reference);
      // }

      // for (const auto& output : function->outputs) {
      //   if (action.contains(json::json_pointer(fmt::format("/outputs/{}", output.identifier)))) {
      //     const std::string variable_name = action["outputs"][output.identifier];
      //     const auto& existing_variable = GetLocalVariable(variable_name);

      //     if (existing_variable.has_value()) {
      //       scope.instructions.push_back(Instruction{
      //           InstructionType::ASSIGN_STACK_VARIABLE,
      //           AssignStackVariable{.source_position = static_cast<int16_t>(
      //                                   -(function->outputs.size() - function->GetOutputIndex(output.identifier))),
      //                               .source_frame = 0,
      //                               .destination_position = static_cast<int16_t>(existing_variable->position),
      //                               .destination_frame = -1}});
      //       scope.instruction_to_action_mappings.push_back(action_reference);
      //     }
      //   }
      // }
      // scope.instructions.push_back(Instruction{ .type = InstructionType::POP_STACK_FRAME });
      // scope.instruction_to_action_mappings.push_back(action_reference);
    }
  }

  void ParseIf(const json& action, std::string path) {
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
      ParsePush(*condition, Type::Get<bool>());

      const std::size_t instruction_count_before = instructions.size();
      instructions.push_back(script_instructions::JumpIfFalse{});
      debug_info.instruction_info.push_back({
        .scope = current_scope,
        .action = path,
      });
      ParseActions(*actions, fmt::format("{}/actions", path));
      const std::size_t instruction_count_after = instructions.size();

      assert(instruction_count_after >= instruction_count_before);
      std::get<script_instructions::JumpIfFalse>(instructions[instruction_count_before]).instruction_offset =
          instruction_count_after - instruction_count_before;
    }
  }

  void ParseWhile(const json& action, std::string path) {
    // auto sub_scope_result = ParseScope(action["actions"], action_reference);
    // if (std::holds_alternative<ScriptError>(sub_scope_result)) {
    //   return std::get<ScriptError>(sub_scope_result);
    // }
    // if (!action.contains("condition")) {
    //   return ScriptError {
    //     .action = action_reference,
    //     .message = "No condition provided",
    //   };
    // }
    // if (!PushValue(action["condition"], &scope)) {
    //   // TODO: Return proper error
    //   return {};
    // }
    // auto& sub_scope = std::get<Scope>(sub_scope_result);
    // if (type == "while") {
    //   sub_scope.instructions.push_back(Instruction{InstructionType::JUMP,
    //       Jump{static_cast<int>(-sub_scope.instructions.size() - 2)}});
    //   sub_scope.instruction_to_action_mappings.push_back(action_reference);
    // }

    // scope.instruction_to_action_mappings.push_back(action_reference);
    // scope.instructions.push_back(Instruction{InstructionType::JUMP_IF_FALSE,
    //                                          Jump{static_cast<int>(sub_scope.instructions.size() + 1)}});
    // scope.instruction_to_action_mappings.push_back(action_reference);

    // scope.instructions.insert(scope.instructions.end(), sub_scope.instructions.begin(), sub_scope.instructions.end());
    // scope.instruction_to_action_mappings.insert(scope.instruction_to_action_mappings.end(),
    //                                             sub_scope.instruction_to_action_mappings.begin(),
    //                                             sub_scope.instruction_to_action_mappings.end());
  }

  void ParsePush(const json& value_definiion, safe_ptr<Type> required_type = nullptr) {}

  std::size_t current_scope;

 public:
  std::vector<Function::ValueDeclaration> inputs;
  std::vector<Function::ValueDeclaration> outputs;
  std::vector<ScriptInstruction> instructions;
  ScriptFunction::DebugInfo debug_info;

  struct Error {
    std::string message;
    std::string path;
  };
  std::vector<Error> errors;
};

}  // namespace

ScriptFunction::ScriptFunction(const json& function_definition) {
  ScriptFunctionParser parser(function_definition);
  assert(parser.errors.size() == 0);

  instructions_ = std::move(parser.instructions);
  inputs_ = std::move(parser.inputs);
  outputs_ = std::move(parser.outputs);
  debug_info_ = std::move(parser.debug_info);
}

void ScriptFunction::Call(std::span<const Value> inputs, std::span<Value> outputs) {
  ScriptExecutionContext execution_context;
  size_t instruction_pointer = 0;

  do {
    std::visit(execution_context, instructions_[instruction_pointer]);
  } while (instruction_pointer < instructions_.size());
}

}  // namespace ovis

