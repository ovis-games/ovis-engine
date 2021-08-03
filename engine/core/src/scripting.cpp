#include <ovis/utils/log.hpp>
#include <ovis/core/scripting.hpp>

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
void print(double x) {
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

#define OVIS_REGISTER_FUNCTION(func, ...) RegisterFunction<decltype(func), func>(#func, __VA_ARGS__);
#define OVIS_REGISTER_FUNCTION_WITH_NAME(func, identifier) RegisterFunction<decltype(func), func>(identifier);

ScriptContext::ScriptContext() {
  types_.push_back(ScriptType(0, "Unknown", false));
  RegisterType<double>("Number");
  RegisterType<bool>("Boolean");
  RegisterType<std::string>("String");

  OVIS_REGISTER_FUNCTION(add, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(subtract, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(multiply, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(divide, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(negate, {"x"}, {"result"});
  OVIS_REGISTER_FUNCTION(print, {"value"});
  OVIS_REGISTER_FUNCTION(print_bool, {"value"});
  OVIS_REGISTER_FUNCTION(is_greater, {"x", "y"}, {"is_greater"});

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

struct Scope {
  const Scope* parent;
  std::map<std::string, int, std::less<>> stack_variable_offsets;
  std::vector<ScriptChunk::Instruction> instructions;
  std::vector<int> instruction_actions;

  std::optional<int> GetStackVariableOffset(std::string_view reference) const {
    const auto offset = stack_variable_offsets.find(reference);
    if (offset != stack_variable_offsets.end()) {
      return offset->second;
    } else if (parent != nullptr) {
      return parent->GetStackVariableOffset(reference);
    } else {
      return {};
    }
  }
};

std::optional<Scope> ParseScope(ScriptContext* context, const json& actions, const Scope* parent_scope = nullptr) {
  using Instruction = ScriptChunk::Instruction;
  using InstructionType = ScriptChunk::InstructionType;
  using PushConstant = ScriptChunk::PushConstant;
  using FunctionCall = ScriptChunk::FunctionCall;
  using PushStackValue = ScriptChunk::PushStackValue;
  using ConditionalJump = ScriptChunk::ConditionalJump;

  int current_stack_offset = 0;
  Scope scope;
  scope.parent = parent_scope;

  auto push_value = [&](const json& definition, Scope* scope) {
    SDL_assert(scope != nullptr);

    if (definition.is_string()) {
      const std::string& definition_string = definition;
      if (definition_string.size() > 0 && definition_string[0] == '$') {
        scope->instructions.push_back(
            Instruction{InstructionType::PUSH_STACK_VARIABLE,
                        PushStackValue{*scope->GetStackVariableOffset(definition_string) - current_stack_offset}});
      } else {
        scope->instructions.push_back(Instruction{
            InstructionType::PUSH_CONSTANT, PushConstant{ScriptValue{{}, double(std::stod(definition_string))}}});
      }
    } else {
      scope->instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
    }
    ++current_stack_offset;

    return true;
  };

  for (const auto& action : actions) {
    const std::string type = action["type"];
    const int action_id = action["id"]; 
    if (type == "function_call") {
      auto function = context->GetFunction(static_cast<std::string>(action["function"]));
      if (!function) {
        LogE("Invalid function name: {}", action["function"]);
        return {};
      }

      for (const auto& output : function->outputs) {
        scope.instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
        scope.instruction_actions.push_back(action_id);
        if (scope.stack_variable_offsets.contains(
                fmt::format("${}:{}", static_cast<int>(action["id"]), output.identifier))) {
          LogE("Duplicate action id: {}", static_cast<size_t>(action["id"]));
          return {};
        }
        scope.stack_variable_offsets[fmt::format("${}:{}", action_id, output.identifier)] = current_stack_offset;
        ++current_stack_offset;
      }

      for (const auto& input : function->inputs) {
        if (!action.contains("inputs") || !action["inputs"].is_object() ||
            !push_value(action["inputs"][input.identifier], &scope)) {
          LogE("No value for input `{}`", input.identifier);
          return {};
        }
        scope.instruction_actions.push_back(action_id);
      }

      // TODO: check input and output count before cast
      scope.instructions.push_back(
          Instruction{InstructionType::FUNCTION_CALL,
                      FunctionCall{static_cast<uint8_t>(function->inputs.size()),
                                   static_cast<uint8_t>(function->outputs.size()), function->function}});
      scope.instruction_actions.push_back(action_id);
      current_stack_offset -= function->inputs.size();
    } else if (type == "if") {
      const auto if_scope = ParseScope(context, action["actions"], &scope);
      if (!if_scope.has_value()) {
        return {};
      }
      if (!push_value(action["condition"], &scope)) {
        return {};
      }
      scope.instruction_actions.push_back(action_id);
      scope.instructions.push_back(Instruction{InstructionType::JUMP_IF_FALSE,
                                               ConditionalJump{static_cast<int>(if_scope->instructions.size() + 1)}});
      scope.instruction_actions.push_back(action_id);
      --current_stack_offset;

      scope.instructions.insert(scope.instructions.end(), if_scope->instructions.begin(), if_scope->instructions.end());
      scope.instruction_actions.insert(scope.instruction_actions.end(), if_scope->instruction_actions.begin(), if_scope->instruction_actions.end());
    }
    assert(scope.instructions.size() == scope.instruction_actions.size());
  }

  return scope;
}

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

ScriptChunk::ScriptChunk(const json& serialized_chunk) {
  instructions_.clear();
  inputs_ = ParseVariableDefinitions(context_, serialized_chunk["inputs"]);
  outputs_ = ParseVariableDefinitions(context_, serialized_chunk["outputs"]);

  const json actions = serialized_chunk["actions"];
  Scope function_scope;
  function_scope.parent = nullptr;
  for (const auto& output : IndexRange(outputs_)) {
    function_scope.stack_variable_offsets[fmt::format("$outputs:{}", output->identifier)] =
        -static_cast<int>(outputs_.size()) - static_cast<int>(inputs_.size()) + output.index();
  }
  for (const auto& input : IndexRange(inputs_)) {
    function_scope.stack_variable_offsets[fmt::format("$inputs:{}", input->identifier)] =
        -static_cast<int>(inputs_.size()) + input.index();
  }
  auto scope = ParseScope(context_, actions, &function_scope);
  if (scope.has_value()) {
    instructions_ = scope->instructions;
    instruction_actions_ = scope->instruction_actions;
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
          return {.error =
                      ScriptError{.action_id = instruction_actions_[instruction_pointer], .message = result->message}};
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
          return ScriptFunctionResult {
            .error = ScriptError {
              .action_id = instruction_actions_[instruction_pointer],
              .message = error.message
            }
          };
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
          return ScriptFunctionResult {
            .error = ScriptError {
              .action_id = instruction_actions_[instruction_pointer],
              .message = error.message
            }
          };
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
