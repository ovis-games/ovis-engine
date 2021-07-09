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
double is_greater(double x, double y) {
  return x > y ? 1.0 : 0.0;
}
void print(double x) {
  LogI("{}", x);
}

}  // namespace

#define OVIS_REGISTER_FUNCTION(func, ...) RegisterFunction<decltype(func), func>(#func, __VA_ARGS__);
#define OVIS_REGISTER_FUNCTION_WITH_NAME(func, identifier) RegisterFunction<decltype(func), func>(identifier);

ScriptContext::ScriptContext() {
  OVIS_REGISTER_FUNCTION(add, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(subtract, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(multiply, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(divide, {"x", "y"}, {"result"});
  OVIS_REGISTER_FUNCTION(negate, {"x"}, {"result"});
  OVIS_REGISTER_FUNCTION(print, {"value"});
  OVIS_REGISTER_FUNCTION(is_greater, {"x", "y"}, {"is_greater"});

  stack_.reserve(1000);
}

void ScriptContext::RegisterFunction(std::string_view identifier, ScriptFunctionPointer function,
                                     std::span<const ScriptVariableDefinition> inputs,
                                     std::span<const ScriptVariableDefinition> outputs) {
  functions_.insert(std::make_pair(
      identifier, ScriptFunction{function, std::vector<ScriptVariableDefinition>{inputs.begin(), inputs.end()},
                                 std::vector<ScriptVariableDefinition>{outputs.begin(), outputs.end()}}));
}

const ScriptFunction* ScriptContext::GetFunction(std::string_view identifier) {
  const auto function = functions_.find(identifier);
  if (function == functions_.end()) {
    return nullptr;
  } else {
    return &function->second;
  }
}

std::variant<ScriptError, std::vector<ScriptVariable>> ScriptContext::Execute(std::string_view function_identifier,
                                                                              std::span<ScriptVariable> inputs) {
  const auto function = functions_.find(function_identifier);

  if (function == functions_.end()) {
    return ScriptError{};
  }

  if (function->second.inputs.size() != inputs.size()) {
    return ScriptError{};
  }

  std::vector<ScriptVariable> outputs(function->second.outputs.size());
  function->second.function(inputs, outputs);

  return outputs;
}

namespace {

struct Scope {
  const Scope* parent;
  std::map<size_t, std::map<std::string, int>> output_offsets;
  std::vector<ScriptChunk::Instruction> instructions;
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
      auto reference = ScriptReference::Parse(definition_string);
      if (reference.has_value()) {
        scope->instructions.push_back(Instruction{
            InstructionType::PUSH_STACK_VARIABLE,
            PushStackValue{scope->output_offsets[reference->action_id][reference->output] - current_stack_offset}});
      } else {
        scope->instructions.push_back(Instruction{
            InstructionType::PUSH_CONSTANT, PushConstant{ScriptVariable{{}, double(std::stod(definition_string))}}});
      }
    } else {
      scope->instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
    }
    ++current_stack_offset;

    return true;
  };

  for (const auto& action : actions) {
    const std::string type = action["type"];
    if (type == "function_call") {
      auto function = context->GetFunction(static_cast<std::string>(action["function"]));
      if (!function) {
        LogE("Invalid function name: {}", action["function"]);
        return {};
      }

      for (const auto& output : function->outputs) {
        scope.instructions.push_back(Instruction{InstructionType::PUSH_CONSTANT, PushConstant{}});
        if (scope.output_offsets.contains(action["id"])) {
          LogE("Duplicate action id: {}", static_cast<size_t>(action["id"]));
          return {};
        }
        scope.output_offsets[action["id"]][output.identifier] = current_stack_offset;
        ++current_stack_offset;
      }

      for (const auto& input : function->inputs) {
        if (!action.contains("inputs") || !action["inputs"].is_object() ||
            !push_value(action["inputs"][input.identifier], &scope)) {
          LogE("No value for input `{}`", input.identifier);
          return {};
        }
      }

      // TODO: check input and output count before cast
      scope.instructions.push_back(
          Instruction{InstructionType::FUNCTION_CALL,
                      FunctionCall{static_cast<uint8_t>(function->inputs.size()),
                                   static_cast<uint8_t>(function->outputs.size()), function->function}});
      current_stack_offset -= function->inputs.size();
    } else if (type == "if") {
      const auto if_scope = ParseScope(context, action["actions"], &scope);
      if (!if_scope.has_value()) {
        return {};
      }
      if (!push_value(action["condition"], &scope)) {
        return {};
      }
      scope.instructions.push_back(Instruction{InstructionType::JUMP_IF_FALSE,
                                               ConditionalJump{static_cast<int>(if_scope->instructions.size() + 1)}});
      --current_stack_offset;

      scope.instructions.insert(scope.instructions.end(), if_scope->instructions.begin(), if_scope->instructions.end());
    }
  }

  return scope;
}

}  // namespace

bool ScriptChunk::Deserialize(const json& serialized_chunk) {
  instructions_.clear();
  const json actions = serialized_chunk["actions"];

  auto scope = ParseScope(context_, actions);
  if (scope.has_value()) {
    instructions_ = scope->instructions;
    return true;
  } else {
    return false;
  }
}

json ScriptChunk::Serialize() const {
  return {};
}

std::variant<ScriptError, std::vector<ScriptVariable>> ScriptChunk::Execute() {
  size_t instruction_pointer = 0;
  while (instruction_pointer < instructions_.size()) {
    const auto& instruction = instructions_[instruction_pointer];

    switch (instruction.type) {
      case InstructionType::FUNCTION_CALL: {
        const auto& function_call = std::get<FunctionCall>(instruction.data);
        function_call.function(
            context_->GetRange(-function_call.input_count, 0),
            context_->GetRange(-(function_call.input_count + function_call.output_count), -function_call.input_count));
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
        context_->PushValue(context_->Get(push_stack_value.position));
        ++instruction_pointer;
        break;
      }

      case InstructionType::JUMP_IF_TRUE: {
        const auto& conditional_jump = std::get<ConditionalJump>(instruction.data);
        if (std::any_cast<double>(context_->Get(-1).value) != 0.0) {
          instruction_pointer += conditional_jump.instruction_offset;
        } else {
          ++instruction_pointer;
        }
        context_->PopStack(1);
        break;
      }

      case InstructionType::JUMP_IF_FALSE: {
        const auto& conditional_jump = std::get<ConditionalJump>(instruction.data);
        if (std::any_cast<double>(context_->Get(-1).value) == 0.0) {
          instruction_pointer += conditional_jump.instruction_offset;
        } else {
          ++instruction_pointer;
        }
        context_->PopStack(1);
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

  return ScriptReference{std::stoull(std::string(reference.substr(1, colon_pos - 1))),
                         std::string(reference.substr(colon_pos + 1))};
}
}  // namespace ovis
