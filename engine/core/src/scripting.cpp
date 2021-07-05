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

  std::vector<int> action_stack_offsets = { 0 };
  std::map<size_t, std::map<std::string, int>> output_offsets;
  int current_stack_offset = 0;

  for (const auto& action_items : IndexRange(actions)) {
    const json& action = action_items.value();
    const std::string type = action["type"];
    if (type == "function_call") {
      // Instruction instruction { InstructionType::FUNCTION_CALL, ;
      auto function = context_->GetFunction(static_cast<std::string>(action["function"]));
      if (!function) {
        LogE("Invalid function name: {}", action["function"]);
        return false;
      }

      for (const auto& output : function->outputs) {
        instructions_.push_back(Instruction{ InstructionType::PUSH_CONSTANT, PushConstant{} });
        output_offsets[action_items.index()][output.identifier] = current_stack_offset;
        ++current_stack_offset;
      }

      for (const auto& input : function->inputs) {
        if (action["inputs"].contains(input.identifier)) {
          const std::string& input_value = action["inputs"][input.identifier];
          if (input_value[0] == '$') {
            const auto colon_pos = input_value.find(':');
            if (colon_pos == std::string::npos) {
              return false;
            }
            const int action_index = std::stoi(input_value.substr(1, colon_pos - 1));
            const std::string output_name = input_value.substr(1, colon_pos - 1);
            instructions_.push_back(Instruction{ InstructionType::PUSH_STACK_VARIABLE, PushStackValue{ output_offsets[action_index][output_name] - current_stack_offset }});
          } else {
            instructions_.push_back(Instruction{ InstructionType::PUSH_CONSTANT, PushConstant{ScriptVariable{ {}, double(std::stod(input_value))}}});
          }
        } else {
          instructions_.push_back(Instruction{ InstructionType::PUSH_CONSTANT, PushConstant{} });
        }
      }

      // TODO: check input and output count before cast
      instructions_.push_back(Instruction{ InstructionType::FUNCTION_CALL, FunctionCall{ static_cast<uint8_t>(function->inputs.size()), static_cast<uint8_t>(function->outputs.size()) }});
    }
  }

  return true;
}

json ScriptChunk::Serialize() const {
  return {};
}

std::variant<ScriptError, std::vector<ScriptVariable>> ScriptChunk::Execute() {
  for (const auto& instruction : instructions_) {
    switch (instruction.type) {
      case InstructionType::FUNCTION_CALL: {
        const auto& function_call = std::get<FunctionCall>(instruction.data);
        function_call.function(context_->GetRange(-function_call.input_count, 0), context_->GetRange(-(function_call.input_count + function_call.output_count), -function_call.input_count));
        context_->PopStack(function_call.input_count);
        break;
      }

      case InstructionType::PUSH_CONSTANT: {
        const PushConstant& push_constant = std::get<PushConstant>(instruction.data);
        context_->PushValue(push_constant.value);
        break;
      }

      case InstructionType::PUSH_STACK_VARIABLE: {
        const PushStackValue& push_stack_value = std::get<PushStackValue>(instruction.data);
        context_->PushValue(context_->Get(push_stack_value.position));
        break;
      }
    };
  }

  return {};
}

}  // namespace ovis
