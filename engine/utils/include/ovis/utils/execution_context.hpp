#pragma once

#include <vector>

#include <ovis/utils/reflection.hpp>

namespace ovis {
class ScriptExecutionContext {
 public:
  ScriptExecutionContext(std::size_t reserved_stack_size = 100) { stack_.reserve(reserved_stack_size); }

 private:
  std::vector<Value> stack_;

  std::size_t GetAbsoluteStackPosition(int position);
};

}

// Implementation
namespace ovis {

inline std::size_t ScriptExecutionContext::operator()(const script_instructions::FunctionCall& function_call) {
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

}

