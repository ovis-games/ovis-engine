#include <ovis/utils/log.hpp>
#include <ovis/core/scripting.hpp>
#include <ovis/core/asset_library.hpp>

namespace ovis {

namespace {

double add(double x, double y) {
  return x + y;
}
double subtract(double x, double y) {
  return x - y;
}
double multiply(double x, double y) {
  return x * y;
}
double divide(double x, double y) {
  return x / y;
}
double negate(double x) {
  return -x;
}
bool is_greater(double x, double y) {
  return x > y;
}
void print(const std::string& text) {
  LogI("{}", text);
}
void print_number(double x) {
  LogI("{}", x);
}
void print_bool(bool x) {
  LogI("{}", x);
}

std::unique_ptr<ScriptContext> global_context = std::make_unique<ScriptContext>();

}  // namespace

ScriptContext* global_script_context() {
  return global_context.get();
}

ScriptContext::ScriptContext() {
  types_.push_back(ScriptType(0, "Unknown", false));
  RegisterType<double>("Number");
  RegisterType<bool>("Boolean");
  RegisterType<std::string>("String");

  RegisterFunction<add>("add", {"x", "y"}, {"result"});
  RegisterFunction<subtract>("subtract", {"x", "y"}, {"result"});
  RegisterFunction<multiply>("multiply", {"x", "y"}, {"result"});
  RegisterFunction<divide>("divide", {"x", "y"}, {"result"});
  RegisterFunction<negate>("negate", {"x"}, {"result"});
  RegisterFunction<print>("print", {"text"}, {"result"});
  RegisterFunction<print_number>("print_number", {"value"}, {"result"});
  RegisterFunction<print_bool>("print_bool", {"value"}, {"result"});
  RegisterFunction<is_greater>("is_greater", {"x", "y"}, {"result"});

  stack_.reserve(1000);
}

void ScriptContext::RegisterFunction(std::string_view identifier, ScriptFunctionPointer function,
                                     std::span<const ScriptValueDefinition> inputs,
                                     std::span<const ScriptValueDefinition> outputs) {
  functions_.insert(std::make_pair(
      identifier, ScriptFunction{function, std::vector<ScriptValueDefinition>{inputs.begin(), inputs.end()},
                                 std::vector<ScriptValueDefinition>{outputs.begin(), outputs.end()}}));
}

const ScriptFunction* ScriptContext::GetFunction(std::string_view identifier) {
  const auto function = functions_.find(identifier);
  if (function == functions_.end()) {
    return nullptr;
  } else {
    return &function->second;
  }
}

bool ScriptContext::LoadDocumentation(std::string_view language) {
  if (!GetEngineAssetLibrary()) {
    return false;
  }

  for (const auto& asset_id : GetEngineAssetLibrary()->GetAssetsWithType("script_documentation")) {
    const auto doc_content = GetEngineAssetLibrary()->LoadAssetTextFile(asset_id, fmt::format("{}.json", language));

    if (!doc_content.has_value()) {
      LogW("Documentation of {} is not available for language {}", asset_id, language);
      continue;
    }

    const auto documentation = json::parse(*doc_content);
    if (!documentation.is_object()) {
      LogE("Invalid documentation format: content must be an object (asset: {}, language: {})", asset_id, language);
      continue;
    }

    if (documentation.contains("functions")) {
      for (const auto& [function_identifier, function_documentation] : documentation["functions"].items()) {
        if (!functions_.contains(function_identifier)) {
          LogW("Found documentation for non-existing fuction `{}` (asset: {}, language: {})", function_identifier,
               asset_id, language);
        } else {
          ScriptFunction* script_function = &functions_[function_identifier];

          if (function_documentation.contains("text")) {
            script_function->text = function_documentation["text"];
            LogD("func text for {}: {}", function_identifier, function_documentation["text"]);
          }
          if (function_documentation.contains("description")) {
            script_function->description = function_documentation["description"];
          }
        }
      }
    }
  }
  
  return true;
}

// std::variant<ScriptError, std::vector<ScriptValue>> ScriptContext::Execute(std::string_view function_identifier,
//                                                                               std::span<ScriptValue> inputs) {
//   const auto function = functions_.find(function_identifier);

//   if (function == functions_.end()) {
//     return ScriptError{};
//   }

//   if (function->second.inputs.size() != inputs.size()) {
//     return ScriptError{};
//   }

//   std::vector<ScriptValue> outputs(function->second.outputs.size());
//   function->second.function(inputs, outputs);

//   return outputs;
// }

namespace {

std::vector<ScriptValueDefinition> ParseVariableDefinitions(ScriptContext* context,
                                                            const json& serialized_definitions) {
  std::vector<ScriptValueDefinition> definitions;
  for (const auto& definition : serialized_definitions.items()) {
    const std::string type(definition.value()["type"]);
    definitions.push_back({context->GetTypeId(type), definition.key()});
  }
  return definitions;
}

}  // namespace

std::variant<ScriptChunk, ScriptError> ScriptChunk::Load(const json& serialized_chunk, ScriptContext* context) {
  ScriptChunk chunk(context);
  chunk.instructions_.clear();
  chunk.inputs_ = ParseVariableDefinitions(context, serialized_chunk["inputs"]);
  chunk.outputs_ = ParseVariableDefinitions(context, serialized_chunk["outputs"]);

  const json actions = serialized_chunk["actions"];
  for (const auto& output : chunk.outputs_) {
    chunk.local_variables_.push_back(LocalVariable{
        .name = output.identifier,
        .declaring_action = ScriptActionReference(),
        .type = output.type,
        .position = static_cast<int>(chunk.local_variables_.size()),
    });
  }
  for (const auto& input : chunk.inputs_) {
    chunk.local_variables_.push_back(LocalVariable{
        .name = input.identifier,
        .declaring_action = ScriptActionReference(),
        .type = input.type,
        .position = static_cast<int>(chunk.local_variables_.size()),
    });
  }
  const auto scope_or_error = chunk.ParseScope(actions);
  if (std::holds_alternative<ScriptError>(scope_or_error)) {
    return std::get<ScriptError>(scope_or_error);
  } else {
    const auto& scope = std::get<Scope>(scope_or_error);
    chunk.instructions_ = scope.instructions;
    chunk.instruction_to_action_mappings_ = scope.instruction_to_action_mappings;
    return chunk;
  }
}

ScriptFunctionResult ScriptChunk::Execute(std::span<const ScriptValue> input_values) {
  if (input_values.size() != inputs_.size()) {
    LogE("Input count does not match");
    return {.error =
                ScriptError{.message = fmt::format("Expected {} inputs, got {}", inputs_.size(), input_values.size())}};
  }

  for (const auto& value : input_values) {
    context_->PushValue(value);
  }

  return Execute();
}

ScriptFunctionResult ScriptChunk::Execute() {
  size_t instruction_pointer = 0;
  while (instruction_pointer < instructions_.size()) {
    const auto& instruction = instructions_[instruction_pointer];

    switch (instruction.type) {
      case InstructionType::FUNCTION_CALL: {
        const auto& function_call = std::get<FunctionCall>(instruction.data);
        auto result = function_call.function(context_, function_call.input_count, function_call.output_count);
        if (result.has_value()) [[unlikely]] {
          return {.error = ScriptError{.action = instruction_to_action_mappings_[instruction_pointer],
                                       .message = result->message}};
        }
        context_->PopStack(function_call.input_count);
        ++instruction_pointer;
        break;
      }

      case InstructionType::PUSH_CONSTANT: {
        const PushConstant& push_constant = std::get<PushConstant>(instruction.data);
        context_->PushValue(push_constant.value);
        ++instruction_pointer;
        break;
      }

      case InstructionType::PUSH_STACK_VARIABLE: {
        const PushStackValue& push_stack_value = std::get<PushStackValue>(instruction.data);
        context_->PushValue(context_->GetValue(push_stack_value.position));
        ++instruction_pointer;
        break;
      }

      case InstructionType::POP: {
        const Pop& pop = std::get<Pop>(instruction.data);
        context_->PopStack(static_cast<std::size_t>(pop.count));
        ++instruction_pointer;
        break;
      }

      case InstructionType::ASSIGN_CONSTANT: {
        const AssignConstant& assign_constant = std::get<AssignConstant>(instruction.data);
        context_->AssignValue(assign_constant.position, assign_constant.value);
        ++instruction_pointer;
        break;
      }

      case InstructionType::ASSIGN_STACK_VARIABLE: {
        const AssignStackVariable& assign_stack_variable = std::get<AssignStackVariable>(instruction.data);
        context_->AssignValue(assign_stack_variable.destination_position,
                              context_->GetValue(assign_stack_variable.source_position));
        break;
      }

      case InstructionType::JUMP_IF_TRUE: {
        const auto& conditional_jump = std::get<ConditionalJump>(instruction.data);
        const auto value_or_error = context_->GetValue<bool>(-1);
        if (std::holds_alternative<bool>(value_or_error)) [[likely]] {
          if (std::get<bool>(value_or_error)) {
            instruction_pointer += conditional_jump.instruction_offset;
          } else {
            ++instruction_pointer;
          }
          context_->PopStack(1);
        } else {
          const auto error = std::get<ScriptError>(value_or_error);
          return ScriptFunctionResult{.error =
                                          ScriptError{.action = instruction_to_action_mappings_[instruction_pointer],
                                                      .message = error.message}};
        }
        break;
      }

      case InstructionType::JUMP_IF_FALSE: {
        const auto& conditional_jump = std::get<ConditionalJump>(instruction.data);
        const auto value_or_error = context_->GetValue<bool>(-1);
        if (std::holds_alternative<bool>(value_or_error)) [[likely]] {
          if (!std::get<bool>(value_or_error)) {
            instruction_pointer += conditional_jump.instruction_offset;
          } else {
            ++instruction_pointer;
          }
          context_->PopStack(1);
        } else {
          const auto error = std::get<ScriptError>(value_or_error);
          return ScriptFunctionResult{.error =
                                          ScriptError{.action = instruction_to_action_mappings_[instruction_pointer],
                                                      .message = error.message}};
        }
        break;
      }
    };
  }

  return {};
}

void ScriptChunk::Print() {
  for (const auto& instruction : instructions_) {
    switch (instruction.type) {
      case InstructionType::FUNCTION_CALL: {
        const auto& function_call = std::get<FunctionCall>(instruction.data);
        LogI("call {} {} {} [-{}]", (void*)function_call.function, function_call.input_count,
             function_call.output_count, function_call.input_count);
        break;
      }

      case InstructionType::PUSH_CONSTANT: {
        const PushConstant& push_constant = std::get<PushConstant>(instruction.data);
        if (push_constant.value.value.has_value()) {
          LogI("push_constant {} [+1]", std::any_cast<double>(push_constant.value.value));
        } else {
          LogI("push_constant None [+1]");
        }
        break;
      }

      case InstructionType::PUSH_STACK_VARIABLE: {
        const PushStackValue& push_stack_value = std::get<PushStackValue>(instruction.data);
        LogI("push_stack_value {} [+1]", push_stack_value.position);
        break;
      }

      case InstructionType::POP: {
        const Pop& pop = std::get<Pop>(instruction.data);
        LogI("pop [-{}]", pop.count);
        break;
      }

      case InstructionType::ASSIGN_CONSTANT: {
        const AssignConstant& assign_constant = std::get<AssignConstant>(instruction.data);
        LogI("assign_constant {}->{} [0]", "some_value", assign_constant.position);
        break;
      }

      case InstructionType::ASSIGN_STACK_VARIABLE: {
        const AssignStackVariable& assign_stack_variable = std::get<AssignStackVariable>(instruction.data);
        LogI("assign_stack_variable {}->{} [0]", assign_stack_variable.source_position,
             assign_stack_variable.destination_position);
        break;
      }

      case InstructionType::JUMP_IF_TRUE: {
        const auto& conditional_jump = std::get<ConditionalJump>(instruction.data);
        LogI("jump_if_true {} [-1]", conditional_jump.instruction_offset);
        break;
      }

      case InstructionType::JUMP_IF_FALSE: {
        const auto& conditional_jump = std::get<ConditionalJump>(instruction.data);
        LogI("jump_if_false {} [-1]", conditional_jump.instruction_offset);
        break;
      }
    }
  }
}

ScriptChunk::ScriptChunk(ScriptContext* context) : context_(context) {}

std::variant<ScriptChunk::Scope, ScriptError> ScriptChunk::ParseScope(const json& actions, const ScriptActionReference& parent) {
  Scope scope;

  auto push_value = [&](const json& value, Scope* scope) {
    // SDL_assert(scope != nullptr);

    if (value.is_string()) {
      const std::string& string = value;
      scope->instructions.push_back(Instruction{
          .type = InstructionType::PUSH_CONSTANT,
          .data = PushConstant{.value = ScriptValue{.type = context_->GetTypeId<std::string>(), .value = string}}});
      return true;
    } else if (value.is_number()) {
      const double number = value;
      scope->instructions.push_back(Instruction{
          .type = InstructionType::PUSH_CONSTANT,
          .data = PushConstant{.value = ScriptValue{.type = context_->GetTypeId<double>(), .value = number}}});
      return true;
    } else if (value.is_boolean()) {
      const bool boolean = value;
      scope->instructions.push_back(Instruction{
          .type = InstructionType::PUSH_CONSTANT,
          .data = PushConstant{.value = ScriptValue{.type = context_->GetTypeId<bool>(), .value = boolean}}});
      return true;
    } else if (value.is_object()) {
      const std::string& local_variable_name = value["local_variable"];
      const auto local_variable = GetLocalVariable(local_variable_name);
      if (!local_variable.has_value()) {
        LogE("Unknown variable: {}", local_variable_name);
        return false;
      } else {
        scope->instructions.push_back(Instruction{.type = InstructionType::PUSH_STACK_VARIABLE,
                                                  .data = PushStackValue{.position = local_variable->position}});
        // TODO: check expected type
        return true;
      }
    } else {
      return false;
    }
  };

  for (const auto& indexed_action : IndexRange(actions)) {
    const json& action = indexed_action.value();
    if (!action.contains("type")) {
      continue;
    }

    const std::string type = action["type"];
    const ScriptActionReference action_reference = parent / indexed_action.index();
    if (type == "function_call") {
      const auto& function_identifier = static_cast<std::string>(action["function"]);
      auto function = context_->GetFunction(function_identifier);
      if (!function) {
        return ScriptError{.action = action_reference,
                           .message = fmt::format("Unknown function identifier '{}'", function_identifier)};
      }

      for (const auto& output : function->outputs) {
        if (action.contains(json::json_pointer(fmt::format("/outputs/{}", output.identifier)))) {
          const std::string variable_name = action["outputs"][output.identifier];
          const auto& existing_variable = GetLocalVariable(variable_name);

          if (!existing_variable.has_value()) {
            scope.instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
            scope.instruction_to_action_mappings.push_back(action_reference);
            local_variables_.push_back(LocalVariable{.name = variable_name,
                                                     .declaring_action = action_reference,
                                                     .type = output.type,
                                                     .position = static_cast<int>(local_variables_.size())});
          } else if (existing_variable->type != output.type) {
            return ScriptError{
                .action = action_reference,
                .message = fmt::format("Assigning function output to local variable with different type. Output '{}' "
                                       "has type '{}', local variable '{}' has type '{}'",
                                       output.identifier, context_->GetType(output.type)->name, variable_name,
                                       context_->GetType(existing_variable->type)->name)};
          }
        }
      }
      for (const auto& output : function->outputs) {
        scope.instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
        scope.instruction_to_action_mappings.push_back(action_reference);
      }

      for (const auto& input : function->inputs) {
        if (!action.contains(json::json_pointer(fmt::format("/inputs/{}", input.identifier))) ||
            !push_value(action["inputs"][input.identifier], &scope)) {
          return ScriptError{.action = action_reference,
                             .message = fmt::format("No value for input '{}' provided", input.identifier)};
        }
        scope.instruction_to_action_mappings.push_back(action_reference);
      }

      // TODO: check input and output count before cast
      scope.instructions.push_back(
          Instruction{InstructionType::FUNCTION_CALL,
                      FunctionCall{static_cast<uint8_t>(function->inputs.size()),
                                   static_cast<uint8_t>(function->outputs.size()), function->function}});
      scope.instruction_to_action_mappings.push_back(action_reference);

      for (const auto& output : function->outputs) {
        if (action.contains(json::json_pointer(fmt::format("/outputs/{}", output.identifier)))) {
          const std::string variable_name = action["outputs"][output.identifier];
          const auto& existing_variable = GetLocalVariable(variable_name);

          if (existing_variable.has_value()) {
            scope.instructions.push_back(Instruction{
                InstructionType::ASSIGN_STACK_VARIABLE,
                AssignStackVariable{.source_position = static_cast<int>(
                                        -(function->outputs.size() - function->GetOutputIndex(output.identifier))),
                                    .destination_position = existing_variable->position}});
            scope.instruction_to_action_mappings.push_back(action_reference);
          }
        }
      }
      scope.instructions.push_back(
          Instruction{.type = InstructionType::POP, .data = Pop{.count = static_cast<int>(outputs_.size())}});
      scope.instruction_to_action_mappings.push_back(action_reference);
    } else if (type == "if") {
      const auto if_scope_result = ParseScope(action["actions"], action_reference);
      if (std::holds_alternative<ScriptError>(if_scope_result)) {
        return std::get<ScriptError>(if_scope_result);
      }
      if (!push_value(action["condition"], &scope)) {
        // TODO: Return proper error
        return {};
      }
      const auto& if_scope = std::get<Scope>(if_scope_result);
      scope.instruction_to_action_mappings.push_back(action_reference);
      scope.instructions.push_back(Instruction{InstructionType::JUMP_IF_FALSE,
                                               ConditionalJump{static_cast<int>(if_scope.instructions.size() + 1)}});
      scope.instruction_to_action_mappings.push_back(action_reference);

      scope.instructions.insert(scope.instructions.end(), if_scope.instructions.begin(), if_scope.instructions.end());
      scope.instruction_to_action_mappings.insert(scope.instruction_to_action_mappings.end(),
                                                  if_scope.instruction_to_action_mappings.begin(),
                                                  if_scope.instruction_to_action_mappings.end());
    }
    assert(scope.instructions.size() == scope.instruction_to_action_mappings.size());
  }

  return scope;
}

std::optional<ScriptChunk::LocalVariable> ScriptChunk::GetLocalVariable(std::string_view name) {
  for (const auto& variable : local_variables_) {
    if (variable.name == name) {
      return variable;
    }
  }
  return {};
}

std::optional<ScriptReference> ScriptReference::Parse(std::string_view reference) {
  if (reference.length() == 0 || reference[0] != '$') {
    return {};
  }

  const auto colon_pos = reference.find(':');
  if (colon_pos == std::string::npos) {
    return {};
  }

  return ScriptReference{std::string(reference.substr(1, colon_pos - 1)), std::string(reference.substr(colon_pos + 1))};
}
}  // namespace ovis
