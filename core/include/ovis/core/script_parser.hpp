#pragma once

#include <string>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/core/function.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/type.hpp>
#include <ovis/core/value.hpp>
#include <ovis/core/virtual_machine.hpp>

namespace ovis {

struct ValueInfo {
  TypeId native_type_id;
  Type::Id vm_type_id;
  bool is_pointer;
};

struct ParseScriptError : Error {
  ParseScriptError(std::string_view message) : Error(message) {}
  ParseScriptError(std::string_view message, std::string_view path) : Error(message), path(path) {}

  std::optional<std::string> path;
};
using ParseScriptErrors = std::vector<ParseScriptError>;

struct ParseScriptFunctionResult {
  std::vector<ValueDeclaration> inputs;
  std::vector<ValueDeclaration> outputs;
  std::vector<vm::Instruction> instructions;
  std::vector<Value> constants;
  // ScriptFunction::DebugInfo debug_info;
};

Result<ParseScriptFunctionResult, ParseScriptErrors> ParseScriptFunction(const json& function_definition);

struct ParseScriptTypeResult {
  TypeDescription type_description;
  std::vector<Value> construct_constants;
  std::vector<vm::Instruction> construct_instructions;
  std::vector<Value> copy_constants;
  std::vector<vm::Instruction> copy_instructions;
  std::vector<Value> destruct_constants;
  std::vector<vm::Instruction> destruct_instructions;
};

Result<ParseScriptTypeResult, ParseScriptErrors> ParseScriptType(const json& type_definition);

// class ScriptFunctionParser {
//   struct ScopeValue {
//     Type::Id type_id;
//     std::optional<std::string> variable_name;
//   };
//   struct Scope {
//     std::vector<ScopeValue> values;
//   };

//  public:

//  private:
//   ScriptFunctionParser(const json& function_definition);
//   // std::vector<ValueDeclaration> ParseInputOutputDeclarations(const json& value_declarations, std::string path);
//   // std::optional<ValueDeclaration> ParseInputOutputDeclaration(const json& value_declaration, std::string path);
//   // void ParseActions(const json& actions, std::string path);
//   // void ParseAction(const json& action, std::string path);
//   // void ParseFunctionCallAction(const json& action, std::string path);
//   // void ParseIf(const json& action, std::string path);
//   // void ParseWhile(const json& action, std::string path);
//   // void ParseReturn(const json& action, std::string path);
//   // void PushNone(const std::string& path);
//   // void ParsePush(const json& value_definiion, const std::string& path, std::shared_ptr<Type> required_type = nullptr, std::size_t stack_frame_offset = 0);
//   // std::optional<std::size_t> GetOutputVariablePosition(std::string_view name, std::shared_ptr<Type> type, const std::string& path);
//   // std::optional<ScriptFunction::DebugInfo::Scope::Variable> GetLocalVariable(std::string_view name);
//   // void PushScope();
//   // void PopScope();

//   std::size_t current_scope_index = std::numeric_limits<std::size_t>::max();
//   ScriptFunction::DebugInfo::Scope& current_scope() { return debug_info.scope_info[current_scope_index]; }

//  public:

//   struct Error {
//     std::string message;
//     std::string path;
//   };
//   std::vector<Error> errors;
// };

}

