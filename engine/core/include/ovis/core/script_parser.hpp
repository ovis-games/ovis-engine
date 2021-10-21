#pragma once

#include <string>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/core/script_instruction.hpp>
#include <ovis/core/script_function.hpp>

namespace ovis {

class ScriptFunctionParser {
 public:
  ScriptFunctionParser(const json& function_definition);

 private:
  std::vector<Function::ValueDeclaration> ParseInputOutputDeclarations(const json& value_declarations);
  void ParseActions(const json& actions, std::string path);
  void ParseAction(const json& action, std::string path);
  void ParseFunctionCallAction(const json& action, std::string path);
  void ParseIf(const json& action, std::string path);
  void ParseWhile(const json& action, std::string path);
  void PushNone(const std::string& path);
  void ParsePush(const json& value_definiion, safe_ptr<Type> required_type = nullptr);
  std::optional<int> GetOutputVariablePosition(std::string_view name, safe_ptr<Type> type, const std::string& path);
  std::optional<ScriptFunction::DebugInfo::Scope::Variable> GetLocalVariable(std::string_view name);

  std::size_t current_scope_index;
  ScriptFunction::DebugInfo::Scope& current_scope() { return debug_info.scope_info[current_scope_index]; }

 public:
  std::vector<Function::ValueDeclaration> inputs;
  std::vector<Function::ValueDeclaration> outputs;
  std::vector<ScriptInstruction> instructions;
  ScriptFunction::DebugInfo debug_info;

  struct Error {
    std::string message;
    std::string path;
  };
  std::vector<Error> errors;
};

}

