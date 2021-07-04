#include <ovis/core/scripting.hpp>
#include <ovis/utils/log.hpp>

namespace ovis {

namespace {

double add(double x, double y) { return x + y; }
double subtract(double x, double y) { return x - y; }
double multiply(double x, double y) { return x * y; }
double divide(double x, double y) { return x / y; }
double negate(double x) { return -x; }
void print(double x) { LogI("{}", x); }

}

#define OVIS_REGISTER_FUNCTION(func) RegisterFunction<decltype(func), func>(#func);
#define OVIS_REGISTER_FUNCTION_WITH_NAME(func, identifier) RegisterFunction<decltype(func), func>(identifier);

ScriptContext::ScriptContext() {
  OVIS_REGISTER_FUNCTION(add);
  OVIS_REGISTER_FUNCTION(subtract);
  OVIS_REGISTER_FUNCTION(multiply);
  OVIS_REGISTER_FUNCTION(divide);
  OVIS_REGISTER_FUNCTION(negate);
  OVIS_REGISTER_FUNCTION(print);

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

bool ScriptChunk::Deserialize(const json& serialized_chunk) {
  instructions_.clear();
  const json actions = serialized_chunk["actions"];

  std::vector<size_t> action_stack_offsets = { 0 };
  size_t current_offset = 0;
  for (const json& action : actions) {
    Instruction instruction;
    auto function = context_->GetFunction(static_cast<std::string>(action["function"]));
    if (!function) {
      LogE("Invalid function name: {}", action["function"]);
      return false;
    }

    current_offset += function->outputs.size();
    action_stack_offsets.push_back(current_offset);

    instruction.function = *function;
    instruction.inputs.resize(function->inputs.size());

    for (const auto& input : action["inputs"].items()) {
      // instruction.inputs.
      auto& value = instruction.inputs[function->GetInputIndex(input.key())];
      const std::string def = input.value();
      if (def[0] == '$') {
        const auto colon_pos = def.find(':');
        const int action_index = std::stoi(colon_pos != std::string::npos ? def.substr(1, colon_pos - 1) : def.substr(1));
        // const std::string output_name = colon_pos != std::string::npos ? def.substr(1, colon_pos - 1) : 
        value = int(current_offset - action_stack_offsets[action_index] + function->inputs.size() - 1);
      } else {
        value = ScriptVariable{ "", double(std::stof(def))};
      }
    }

    instructions_.push_back(instruction);
  }

  return true;
}

json ScriptChunk::Serialize() const {
  return {};
}

std::variant<ScriptError, std::vector<ScriptVariable>> ScriptChunk::Execute() {
  for (const auto& instruction : instructions_) {
    auto outputs = context_->PushStack(instruction.function.outputs.size());
    auto inputs = context_->PushStack(instruction.function.inputs.size());
    for (const auto& input : IndexRange(instruction.inputs)) {
      if (std::holds_alternative<int>(*input)) {
        inputs[input.index()] = context_->Get(std::get<int>(*input));
      } else if (std::holds_alternative<ScriptVariable>(*input)) {
        inputs[input.index()] = std::get<ScriptVariable>(*input);
      }
    }
    instruction.function.function(inputs, outputs);
    context_->PopStack(inputs.size());
  }

  return {};
}

}  // namespace ovis
