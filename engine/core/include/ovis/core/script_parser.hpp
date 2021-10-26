#pragma once

#include <string>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/core/script_instruction.hpp>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/core/script_function.hpp>

namespace ovis {

class ScriptFunctionParser {
 public:
  ScriptFunctionParser(const json& function_definition);

 private:
  std::vector<vm::Function::ValueDeclaration> ParseInputOutputDeclarations(const json& value_declarations, std::string path);
  std::optional<vm::Function::ValueDeclaration> ParseInputOutputDeclaration(const json& value_declaration, std::string path);
  void ParseActions(const json& actions, std::string path);
  void ParseAction(const json& action, std::string path);
  void ParseFunctionCallAction(const json& action, std::string path);
  void ParseIf(const json& action, std::string path);
  void ParseWhile(const json& action, std::string path);
  void ParseReturn(const json& action, std::string path);
  void PushNone(const std::string& path);
  void ParsePush(const json& value_definiion, const std::string& path, safe_ptr<vm::Type> required_type = nullptr, std::size_t stack_frame_offset = 0);
  std::optional<std::size_t> GetOutputVariablePosition(std::string_view name, safe_ptr<vm::Type> type, const std::string& path);
  std::optional<ScriptFunction::DebugInfo::Scope::Variable> GetLocalVariable(std::string_view name);
  void PushScope();
  void PopScope();

  std::size_t current_scope_index = std::numeric_limits<std::size_t>::max();
  ScriptFunction::DebugInfo::Scope& current_scope() { return debug_info.scope_info[current_scope_index]; }

 public:
  std::vector<vm::Function::ValueDeclaration> inputs;
  std::vector<vm::Function::ValueDeclaration> outputs;
  std::vector<vm::Instruction> instructions;
  ScriptFunction::DebugInfo debug_info;

  struct Error {
    std::string message;
    std::string path;
  };
  std::vector<Error> errors;
};

}

