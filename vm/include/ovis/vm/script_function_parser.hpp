#pragma once

#include <string>
#include <vector>
#include <deque>

#include "ovis/utils/json.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/function.hpp"
#include "ovis/vm/type.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/parse_script_error.hpp"
#include "schemas/function.hpp"

namespace ovis {

struct ParseScriptFunctionResult {
  FunctionDescription function_description;
  // ScriptFunction::DebugInfo debug_info;
  ParseScriptErrors errors;
};

ParseScriptFunctionResult ParseScriptFunction(VirtualMachine* virtual_machine, const json& function_definition,
                                              std::string_view script_name = "", std::string_view base_path = "/");

struct ScriptFunctionScopeValue {
  TypeId type_id;
  std::optional<std::string> name;
  uint32_t index;
};

struct ScriptFunctionScope {
  ScriptFunctionScope * parent = nullptr;
  uint32_t base_index;
  std::deque<ScriptFunctionScopeValue> values;

  ScriptFunctionScopeValue* GetValue(std::uint32_t index);
  const ScriptFunctionScopeValue* GetVariable(std::string_view name);
  Result<uint32_t, ParseScriptError> AddVariable(TypeId type, std::string_view name);

  NotNull<ScriptFunctionScopeValue*> PushValue(TypeId type);
  uint32_t PopValue();
  uint32_t current_stack_offset() { return base_index + values.size(); }
};

struct ScriptFunctionParser {
  VirtualMachine* virtual_machine;
  std::string script_name;
  std::string base_path;
  std::deque<ScriptFunctionScope> scopes;
  ParseScriptFunctionResult result;
  ScriptFunctionDefinition& definition;

  ScriptFunctionParser(VirtualMachine* virtual_machine, std::string_view script_name, std::string_view base_path)
      : virtual_machine(virtual_machine),
        script_name(script_name),
        base_path(base_path),
        result(
            {.function_description = {.virtual_machine = virtual_machine, .definition = ScriptFunctionDefinition{}}}),
        definition(std::get<1>(result.function_description.definition)) {}

  template <typename... FormatArguments>
  void AddError(std::string_view path, std::string_view error_message, FormatArguments&&... format_arguments) {
    result.errors.emplace_back(ScriptErrorLocation(script_name, path), error_message, std::forward<FormatArguments>(format_arguments)...);
  }

  void Parse(const schemas::Function& function);
  void ParseOutputs(const std::vector<schemas::VariableDeclaration>& outputs, std::string_view path);
  void ParseInputs(const std::vector<schemas::VariableDeclaration>& inputs, std::string_view path);

  // Statement parsing
  void ParseStatements(const std::vector<schemas::Statement>& statements, std::string_view path);
  void ParseStatement(const schemas::Statement& statement, std::string_view path);
  void ParseReturnStatement(const schemas::Expression& return_expression, std::string_view path);
  void ParseExpressionStatement(const schemas::Expression& expression, std::string_view path);
  void ParseVariableDeclarationStatement(const schemas::VariableDeclarationStatement& variable_declaration_statement,
                                         std::string_view path);

  // Expression parsing
  std::span<ScriptFunctionScopeValue> ParseExpression(const schemas::Expression& expression_definition, std::string_view path);
  std::span<ScriptFunctionScopeValue> ParseConstantExpression(const schemas::ConstantExpression& constant_expression, std::string_view path);
  std::span<ScriptFunctionScopeValue> ParseFunctionCallExpression(const schemas::FunctionCallExpression& function_call_expression, std::string_view path);
  std::span<ScriptFunctionScopeValue> ParseVariableExpression(const std::string& variable, std::string_view path);
  std::span<ScriptFunctionScopeValue> ParseOperatorExpression(const schemas::OperatorExpression& operator_expression, std::string_view path);
  std::span<ScriptFunctionScopeValue> ParseInfixOperatorOperands(const schemas::OperatorExpression& operator_expression, std::string_view path);
  bool CheckValueTypes(std::span<ScriptFunctionScopeValue> values, const std::vector<TypeId>& expected_types, std::string_view path, std::string_view error_message = "");

  void ParseFunctionCall(const json& statement_definiton, std::string_view path);
  void ParsePushValue(const json& value_definition, std::string_view path, TypeId type);
  void ParsePushVariable(const json& value_definition, std::string_view path, TypeId type);
  void ParsePushVariableReference(const json& value_definition, std::string_view path, TypeId type);

  void CallFunction(const std::shared_ptr<Function> function);

  ScriptFunctionScope* PushScope();
  void PopScope();
  ScriptFunctionScope* current_scope();

  template <typename T> std::uint32_t InsertConstant(T&& value);
  template <typename T> ScriptFunctionScopeValue* InsertPushConstantInstructions(std::string_view path, T&& value);
  ScriptFunctionScopeValue* InsertConstructTypeInstructions(std::string_view path, NotNull<const Type*> type);
  void InsertPopValueInstructions(std::string_view path, std::size_t count);
  void InsertCopyInstructions(std::string_view path, NotNull<const Type*> type, uint32_t destination_index, uint32_t source_index);
  void InsertAssignInstructions(std::string_view path, NotNull<const Type*> type, uint32_t destination_index);
  void InsertPrepareFunctionCallInstructions(std::string_view path, NotNull<const Function*> function);
  void InsertFunctionCallInstructions(std::string_view path, NotNull<const Function*> function);
  void InsertInstructions(std::string_view path, std::initializer_list<Instruction> instructions);
};

}  // namespace ovis
