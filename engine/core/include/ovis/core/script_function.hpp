#pragma once

#include <optional>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/core/asset_library.hpp>

namespace ovis {

class ScriptFunctionParser ;

class ScriptFunction {
 public:
  struct DebugInfo {
    struct Scope {
      struct Variable {
        vm::Function::ValueDeclaration declaration;
        std::size_t position;
      };
      std::size_t parent_scope;
      std::vector<Variable> variables;
      int position_offset;
    };
    struct Instruction {
      std::size_t scope;
      std::string action;
    };

    std::vector<Instruction> instruction_info;
    std::vector<Scope> scope_info;
  };

  ScriptFunction(const ScriptFunctionParser& parser);
  ScriptFunction(const json& function_definition);

  std::span<const vm::Function::ValueDeclaration> inputs() const { return inputs_; }
  std::span<const vm::Function::ValueDeclaration> outputs() const { return outputs_; }
  const DebugInfo* debug_info() const { return &debug_info_; }

  template <typename... OutputTypes, typename... InputsTypes>
  vm::FunctionResultType<OutputTypes...> Call(InputsTypes&&... inputs);
  template <typename... OutputTypes, typename... InputsTypes>
  vm::FunctionResultType<OutputTypes...> Call(vm::ExecutionContext* context, InputsTypes&&... inputs);

 private:
  std::vector<vm::Function::ValueDeclaration> inputs_;
  std::vector<vm::Function::ValueDeclaration> outputs_;
  std::vector<vm::Instruction> instructions_;
  DebugInfo debug_info_;

  void Execute(vm::ExecutionContext* context = vm::ExecutionContext::global_context());
};

ScriptFunction LoadScriptFunction(std::string_view asset_id, std::string_view asset_file);
ScriptFunction LoadScriptFunction(AssetLibrary* asset_library, std::string_view asset_id, std::string_view asset_file);

}  // namespace ovis


// Implementation
namespace ovis {
template <typename... OutputTypes, typename... InputTypes>
inline vm::FunctionResultType<OutputTypes...> ScriptFunction::Call(InputTypes&&... inputs) {
  return Call<OutputTypes...>(vm::ExecutionContext::global_context(), std::forward<InputTypes>(inputs)...);
}

template <typename... OutputTypes, typename... InputTypes>
inline vm::FunctionResultType<OutputTypes...> ScriptFunction::Call(vm::ExecutionContext* context, InputTypes&&... inputs) {
  assert(sizeof...(InputTypes) == inputs_.size());
  // TODO: validate input/output types
  context->PushStackFrame();
  ((context->PushValue(std::forward<InputTypes>(inputs))), ...);
  Execute();
  if constexpr (sizeof...(OutputTypes) == 0) {
    context->PopStackFrame();
  } else {
    auto result = context->GetTopValue<vm::FunctionResultType<OutputTypes...>>();
    context->PopStackFrame();
    return result;
  }
}
}

