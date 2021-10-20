#pragma once

#include <optional>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/reflection.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/script_instruction.hpp>

namespace ovis {

class ScriptFunction {
 public:
  struct DebugInfo {
    struct Scope {
      std::size_t parent_scope;
      std::vector<Function::ValueDeclaration> variables;
    };
    struct Instruction {
      std::size_t scope;
      std::string action;
    };

    std::vector<Instruction> instruction_info;
    std::vector<Scope> scope_info;
  };

  ScriptFunction(const json& function_definition);

  std::span<const Function::ValueDeclaration> inputs() const { return inputs_; }
  std::span<const Function::ValueDeclaration> outputs() const { return outputs_; }
  const DebugInfo* debug_info() const { return &debug_info_; }

  void Call(std::span<const Value> inputs, std::span<Value> outputs);

 private:
  std::vector<Function::ValueDeclaration> inputs_;
  std::vector<Function::ValueDeclaration> outputs_;
  std::vector<ScriptInstruction> instructions_;
  DebugInfo debug_info_;
};

ScriptFunction LoadScriptFunction(std::string_view asset_id, std::string_view asset_file);
ScriptFunction LoadScriptFunction(AssetLibrary* asset_library, std::string_view asset_id, std::string_view asset_file);

}  // namespace ovis

