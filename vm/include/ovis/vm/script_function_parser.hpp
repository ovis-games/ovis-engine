#pragma once

#include <string>
#include <vector>
#include <deque>

#include <ovis/utils/json.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/vm/function.hpp>
#include <ovis/vm/type.hpp>
#include <ovis/vm/value.hpp>
#include <ovis/vm/parse_script_error.hpp>

namespace ovis {

struct ParseScriptFunctionResult {
  FunctionDescription function_description;
  // ScriptFunction::DebugInfo debug_info;
};

Result<ParseScriptFunctionResult, ParseScriptErrors> ParseScriptFunction(VirtualMachine* virtual_machine, const json& function_definition);

struct ScriptFunctionScopeValue {
  TypeId type_id;
  std::optional<std::string> name;
  uint32_t index;
};

struct ScriptFunctionScope {
  ScriptFunctionScope * parent = nullptr;
  uint32_t base_index;
  std::vector<ScriptFunctionScopeValue> values;

  const ScriptFunctionScopeValue* GetVariable(std::string_view name);
  Result<uint32_t, ParseScriptError> AddVariable(TypeId type, std::string_view name);

  NotNull<ScriptFunctionScopeValue*> PushValue(TypeId type);
  uint32_t PopValue();
  uint32_t current_stack_offset() { return base_index + values.size(); }
};

struct ScriptFunctionParser {
  VirtualMachine* virtual_machine;
  std::deque<ScriptFunctionScope> scopes;
  ParseScriptFunctionResult result;
  ScriptFunctionDefinition& definition;
  ParseScriptErrors errors;

  ScriptFunctionParser(VirtualMachine* virtual_machine)
      : virtual_machine(virtual_machine),
        result(
            {.function_description = {.virtual_machine = virtual_machine, .definition = ScriptFunctionDefinition{}}}),
        definition(std::get<1>(result.function_description.definition)) {}

  void Parse(const json& function_definition);
  void ParseOutputs(const json& outputs, std::string_view path);
  void ParseInputs(const json& inputs, std::string_view path);
  void ParseStatements(const json& statement_definiton, std::string_view path);
  void ParseStatement(const json& statement_definiton, std::string_view path);
  void ParseReturn(const json& return_definition, std::string_view path);
  void ParseVariableDeclaration(const json& statement_definiton, std::string_view path);

  TypeId ParseExpression(const json& expression_definition, std::string_view path);
  void ParseFunctionCall(const json& statement_definiton, std::string_view path);
  void ParsePushValue(const json& value_definition, std::string_view path, TypeId type);
  void ParsePushVariable(const json& value_definition, std::string_view path, TypeId type);
  void ParsePushVariableReference(const json& value_definition, std::string_view path, TypeId type);

  void CallFunction(const std::shared_ptr<Function> function);

  ScriptFunctionScope* PushScope();
  void PopScope();
  ScriptFunctionScope* current_scope();

  template <typename T> std::uint32_t InsertConstant(T&& value);
  template <typename T> void InsertPushConstantInstructions(std::string_view path, T&& value);
  ScriptFunctionScopeValue* InsertConstructTypeInstruction(std::string_view path, NotNull<const Type*> type);
  void InsertAssignInstructions(std::string_view path, NotNull<const Type*> type, uint32_t destination_index);
  void InsertPrepareFunctionCallInstructions(std::string_view path, NotNull<const Function*> function);
  void InsertFunctionCallInstructions(std::string_view path, NotNull<const Function*> function);
  void InsertInstructions(std::string_view path, std::initializer_list<Instruction> instructions);
};

}  // namespace ovis
