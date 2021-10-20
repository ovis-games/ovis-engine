#pragma once

#include <vector>

#include <ovis/utils/reflection.hpp>
#include <ovis/core/script_instruction.hpp>

namespace ovis {

class ScriptExecutionContext {
 public:
   void operator()(const script_instructions::FunctionCall& function_call);
   void operator()(const script_instructions::PushConstant& push_constant);
   void operator()(const script_instructions::AssignConstant& assign_constant);
   void operator()(const script_instructions::AssignStackVariable& assign_stack_variable);
   void operator()(const script_instructions::Pop& pop);
   void operator()(const script_instructions::JumpIfTrue& jump);
   void operator()(const script_instructions::JumpIfFalse& jump);

 private:
  std::vector<Value> stack_;
};

}

// Implementation
namespace ovis {

inline void ScriptExecutionContext::operator()(const script_instructions::FunctionCall& function_call) {
}

}

