#include <ovis/core/script_function.hpp>
#include <ovis/core/script_execution_context.hpp>
#include <ovis/core/script_parser.hpp>

namespace ovis {

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

