#include <ovis/core/script_function.hpp>
#include <ovis/core/script_parser.hpp>

namespace ovis {

ScriptFunction::ScriptFunction(const ScriptFunctionParser& parser) {
  assert(parser.errors.size() == 0);
  instructions_ = std::move(parser.instructions);
  inputs_ = std::move(parser.inputs);
  outputs_ = std::move(parser.outputs);
  debug_info_ = std::move(parser.debug_info);
}

ScriptFunction::ScriptFunction(const json& function_definition)
    : ScriptFunction(ScriptFunctionParser(function_definition)) {}

void ScriptFunction::Execute(vm::ExecutionContext* context) {
  // size_t instruction_pointer = 0;

  // do {
  //   const auto instruction_pointer_offset = std::visit(*context, instructions_[instruction_pointer]);
  //   if (instruction_pointer_offset.has_value()) {
  //     instruction_pointer += *instruction_pointer_offset;
  //   } else {
  //     return;
  //   }
  // } while (instruction_pointer < instructions_.size());

  assert("Missing return statement");
}

}  // namespace ovis

