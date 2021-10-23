#include <ovis/core/script_function.hpp>
#include <ovis/utils/execution_context.hpp>
#include <ovis/core/script_parser.hpp>

  // std::size_t operator()(const script_instructions::FunctionCall& function_call);
  // std::size_t operator()(const script_instructions::PushConstant& push_constant);
  // std::size_t operator()(const script_instructions::PushStackValue& push_stack_value);
  // std::size_t operator()(const script_instructions::AssignConstant& assign_constant);
  // std::size_t operator()(const script_instructions::AssignStackValue& assign_stack_value);
  // std::size_t operator()(const script_instructions::Pop& pop);
  // std::size_t operator()(const script_instructions::Jump& jump);
  // std::size_t operator()(const script_instructions::JumpIfTrue& jump);
  // std::size_t operator()(const script_instructions::JumpIfFalse& jump);
/*inline std::size_t ScriptExecutionContext::operator()(const script_instructions::FunctionCall& function_call) {
  assert(stack_.size() >= function_call.input_count + function_call.output_count);
  std::span<Value> outputs(stack_.data() + stack_.size() - function_call.input_count - function_call.output_count,
                           function_call.output_count);
  std::span<const Value> inputs(stack_.data() + stack_.size() - function_call.input_count, function_call.input_count);
  function_call.function_pointer(inputs, outputs);
  return 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::PushConstant& push_constant) {
  stack_.push_back(push_constant.value);
  return 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::PushStackValue& push_stack_value) {
  stack_.push_back(stack_[GetAbsoluteStackPosition(push_stack_value.position)]);
  return 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::AssignConstant& assign_constant) {
  stack_[assign_constant.position] = assign_constant.value;
  return 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::AssignStackValue& assign_stack_variable) {
  stack_[GetAbsoluteStackPosition(assign_stack_variable.destination_position)] =
      stack_[GetAbsoluteStackPosition(assign_stack_variable.source_position)];
  return 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::Pop& pop) {
  assert(pop.count <= stack_.size());
  stack_.erase(stack_.end() - pop.count, stack_.end());
  return 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::Jump& jump) {
  return jump.instruction_offset;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::JumpIfTrue& jump) {
  const bool condition = stack_.back().Get<bool>();
  stack_.pop_back();
  return condition ? jump.instruction_offset : 1;
}

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::JumpIfFalse& jump) {
  const bool condition = stack_.back().Get<bool>();
  stack_.pop_back();
  return condition ? 1 : jump.instruction_offset;
}

inline std::size_t ScriptExecutionContext::GetAbsoluteStackPosition(int position) {
  return position >= 0 ? position : stack_.size() - position;
}
*/


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

