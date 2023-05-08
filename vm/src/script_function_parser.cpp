#include "ovis/vm/script_function_parser.hpp"

#include <deque>
#include <variant>

#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

ParseScriptFunctionResult ParseScriptFunction(VirtualMachine* virtual_machine, const json& function_definition,
                                              std::string_view script_name, std::string_view base_path) {
  ScriptFunctionParser parser(virtual_machine, script_name, base_path);
  parser.Parse(function_definition);
  return parser.result;
}

const ScriptFunctionScopeValue* ScriptFunctionScope::GetVariable(std::string_view name) {
  for (const auto& variable : values) {
    if (variable.name == name) {
      return &variable;
    }
  }
  return parent ? parent->GetVariable(name) : nullptr;
}

Result<uint32_t, ParseScriptError> ScriptFunctionScope::AddVariable(TypeId type, std::string_view name) {
  if (name.length() > 0 && GetVariable(name)) {
    return ParseScriptError(std::nullopt, "Duplicate variable name: {}", name);
  }
  values.push_back(ScriptFunctionScopeValue{
    .type_id = type,
    .name = name.length() > 0 ? std::optional(std::string(name)) : std::nullopt,
    .index = static_cast<uint32_t>(base_index + values.size()),
  });
  return values.back().index;
}


NotNull<ScriptFunctionScopeValue*> ScriptFunctionScope::PushValue(TypeId type) {
  values.push_back({
    .type_id = type,
    .index = static_cast<uint32_t>(values.size()),
  });
  return &values.back();
}

uint32_t ScriptFunctionScope::PopValue() {
  const auto index = values.back().index;
  values.pop_back();
  return index;
}

void ScriptFunctionParser::Parse(const schemas::Function& function) {
  result.function_description.name = function.name;
  PushScope();
  ParseOutputs(function.outputs, fmt::format("{}/outputs", base_path));
  current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  ParseInputs(function.inputs, fmt::format("{}/inputs", base_path));
  ParseStatements(function.statements, fmt::format("{}/statments", base_path));
  // Insert an implicit return at the end of the function
  InsertInstructions(base_path, {
    Instruction::CreateReturn(result.function_description.outputs.size()),
  });
}

void ScriptFunctionParser::ParseOutputs(const std::vector<schemas::Variable>& outputs, std::string_view path) {
  for (const auto& output : outputs) {
    const auto type_id = virtual_machine->GetTypeId(output.type);
    current_scope()->AddVariable(type_id, "");
    result.function_description.outputs.push_back({ .name = output.name, .type = type_id });
  }
}

void ScriptFunctionParser::ParseInputs(const std::vector<schemas::Variable>& inputs, std::string_view path) {
  for (const auto& input : inputs) {
    const auto type_id = virtual_machine->GetTypeId(input.type);
    auto add_variable_result = current_scope()->AddVariable(type_id, input.name);
    if (add_variable_result) {
      result.function_description.inputs.push_back({.name = input.name, .type = type_id});
    } else {
      AddError(path, add_variable_result.error().message);
    }
  }
}

void ScriptFunctionParser::ParseStatements(const std::vector<schemas::StatementSchema>& statements, std::string_view path) {
  for (const auto& statement : IndexRange(statements)) {
    ParseStatement(*statement, fmt::format("{}/{}", path, statement.index()));
  }
}

void ScriptFunctionParser::ParseStatement(const schemas::StatementSchema& statement, std::string_view path) {
  switch (statement.type) {
    case schemas::StatementType::RETURN:
      assert(statement.statement_schema_return);
      ParseReturnStatement(*statement.statement_schema_return.get(), path);
      return;

    case schemas::StatementType::VARIABLE_DECLARATION:
      assert(statement.variable_declaration);
      ParseVariableDeclarationStatement(*statement.variable_declaration, path);
      return;

    case schemas::StatementType::EXPRESSION:
      assert(statement.expression);
      ParseExpressionStatement(*statement.expression, path);
      return;

    default:
      AddError(path, "Invalid statement type.");
  }
}

void ScriptFunctionParser::ParseReturnStatement(const schemas::Expression& return_expression, std::string_view path) {
  std::vector<TypeId> output_types;
  output_types.reserve(result.function_description.outputs.size());
  for (const auto& output : result.function_description.outputs) {
    output_types.push_back(output.type);
  }
  if (!CheckValueTypes(ParseExpression(return_expression, path), output_types, path, "Incorrect types passed to return statement.")) {
    return;
  }
  for (int i = output_types.size() - 1; i >= 0; --i) {
    InsertAssignInstructions(path, virtual_machine->GetType(output_types[i]), i);
  }
  InsertInstructions(path, {
    Instruction::CreateReturn(result.function_description.outputs.size()),
  });
}

void ScriptFunctionParser::ParseExpressionStatement(const schemas::Expression& expression, std::string_view path) {
  auto values = ParseExpression(expression, path);
  InsertPopValueInstructions(path, values.size());
}

void ScriptFunctionParser::ParseVariableDeclarationStatement(
    const schemas::VariableDeclaration& variable_declaration_statement, std::string_view path) {
  const auto type = virtual_machine->GetType(variable_declaration_statement.variable.type);
  if (!type) {
    AddError(path, "Unknown variable type {}", variable_declaration_statement.variable.type);
    return;
  }

  ScriptFunctionScopeValue* value;
  if (variable_declaration_statement.value) {
    auto expression_values = ParseExpression(*variable_declaration_statement.value, fmt::format("{}/variable/value", path));
    if (expression_values.size() != 1) {
      AddError(path, "The expression initializing the variable needs to produce exactly one value");
      return;
    }
    value = &expression_values[0];
    if (value->type_id != type->id()) {
      AddError(path, "Invalid expression type. Expected {}, got {}.", type->name(),
               virtual_machine->GetType(value->type_id)->name());
    }
  } else {
    value = InsertConstructTypeInstructions(path, type);
  }
  if (value) {
    value->name = variable_declaration_statement.variable.name;
  }
}

std::span<ScriptFunctionScopeValue> ScriptFunctionParser::ParseExpression(const schemas::Expression& expression_definition,
                                                                std::string_view path) {
  switch (expression_definition.type) {
    case schemas::ExpressionType::CONSTANT:
      assert(expression_definition.constant);
      return ParseConstantExpression(*expression_definition.constant, fmt::format("{}/constant", path));

    case schemas::ExpressionType::FUNCTION_CALL:
      assert(expression_definition.function_call);
      return ParseFunctionCallExpression(*expression_definition.function_call, fmt::format("{}/functionCall", path));

    case schemas::ExpressionType::OPERATOR:
      assert(expression_definition.expression_operator);
      return ParseOperatorExpression(*expression_definition.expression_operator, fmt::format("{}/operator", path));

    case schemas::ExpressionType::VARIABLE:
      assert(expression_definition.variable);
      return ParseVariableExpression(*expression_definition.variable, fmt::format("{}/variable", path));

    default:
      AddError(path, "Unknown expression");
      return {};
  };
}

std::span<ScriptFunctionScopeValue> ScriptFunctionParser::ParseConstantExpression(const schemas::Constant& constant_expression, std::string_view path) {
  return std::visit(
      [this, path](const auto& value) {
        return std::span<ScriptFunctionScopeValue>{InsertPushConstantInstructions(path, value), 1};
      },
      constant_expression);
}

std::span<ScriptFunctionScopeValue> ScriptFunctionParser::ParseFunctionCallExpression(
    const schemas::FunctionCall& function_call_expression, std::string_view path) {
  const auto function = virtual_machine->GetFunction(function_call_expression.function);
  if (!function) {
    AddError(path, "Unknown function: {}", function_call_expression.function);
    return {};
  }
  if (function->inputs().size() != function_call_expression.inputs.size()) {
    AddError(path, "Incorrect number of inputs. Expected: {}, got {}.", function->inputs().size(),
             function_call_expression.inputs.size());
    return {};
  }
  InsertPrepareFunctionCallInstructions(path, function);
  for (int i = 0; i < function_call_expression.inputs.size(); ++i) {
    const auto input_path = fmt::format("{}/inputs/{}", path, i);
    const auto error_messsage = fmt::format("Incorrect type for input {}: {}.", i, function->inputs()[i].name);

    CheckValueTypes(ParseExpression(function_call_expression.inputs[i], input_path), { function->inputs()[i].type }, input_path, error_messsage);
  }
  InsertFunctionCallInstructions(path, function);
  for (std::size_t i = 0; i < function->outputs().size(); ++i) {
    const std::size_t scope_index = current_scope()->values.size() - function->outputs().size() + i;
    assert(current_scope()->values[scope_index].type_id == function->outputs()[i].type);
  }
  return {&current_scope()->values[current_scope()->values.size() - function->outputs().size()],
          function->outputs().size()};
}

std::span<ScriptFunctionScopeValue> ScriptFunctionParser::ParseVariableExpression(const std::string& variable_name, std::string_view path) {
  const auto variable = current_scope()->GetVariable(variable_name);
  if (!variable) {
    AddError(path, "Unknown variable: {}", variable_name);
    return {};
  }

  const auto type = virtual_machine->GetType(variable->type_id);
  auto new_value = InsertConstructTypeInstructions(path, type);
  InsertCopyInstructions(path, type, new_value->index, variable->index);
  return {new_value, 1};
}

std::span<ScriptFunctionScopeValue> ScriptFunctionParser::ParseOperatorExpression(
    const schemas::OperatorClass& operator_expression, std::string_view path) {
  const auto number_type = virtual_machine->GetTypeId<double>();
  const auto boolean_type = virtual_machine->GetTypeId<bool>();

  switch (operator_expression.operator_operator) {
    case schemas::OperatorEnum::ADD:
      if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {number_type, number_type}, path)) {
        return {};
      }
      InsertInstructions(path, { Instruction::CreateAddNumbers() });
      current_scope()->PopValue();
      assert(current_scope()->values.back().type_id == number_type);
      assert(!current_scope()->values.back().name.has_value());
      return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::SUBTRACT:
      if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {number_type, number_type}, path)) {
        return {};
      }
      InsertInstructions(path, { Instruction::CreateSubtractNumbers() });
      current_scope()->PopValue();
      return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::MULTIPLY:
      if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {number_type, number_type}, path)) {
        return {};
      }
      InsertInstructions(path, { Instruction::CreateMultiplyNumbers() });
      current_scope()->PopValue();
      return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::DIVIDE:
      if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {number_type, number_type}, path)) {
        return {};
      }
      InsertInstructions(path, { Instruction::CreateDivideNumbers() });
      current_scope()->PopValue();
      return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::NEGATE:
      AddError(path, "Negate operator is not implemented yet");
      return {};

    case schemas::OperatorEnum::AND:
      AddError(path, "And operator is not implemented yet");
      return {};
      // if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {boolean_type, boolean_type}, path)) {
      //   return {};
      // }
      // InsertInstructions(path, { Instruction::CreateAnd() });
      // current_scope()->PopValue();
      // return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::OR:
      AddError(path, "Or operator is not implemented yet");
      return {};
      // if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {boolean_type, boolean_type}, path)) {
      //   return {};
      // }
      // InsertInstructions(path, { Instruction::CreateOr() });
      // current_scope()->PopValue();
      // return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::NOT:
      AddError(path, "Not operator is not implemented yet");
      return {};

    case schemas::OperatorEnum::EQUALS:
      AddError(path, "Equals operator is not implemented yet");
      return {};

    case schemas::OperatorEnum::NOT_EQUALS:
      AddError(path, "Not-equale operator is not implemented yet");
      return {};

    case schemas::OperatorEnum::GREATER:
      if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {number_type, number_type}, path)) {
        return {};
      }
      InsertInstructions(path, { Instruction::CreateIsNumberGreater() });
      current_scope()->PopValue();
      current_scope()->values.back().type_id = boolean_type;
      assert(!current_scope()->values.back().name.has_value());
      return {&current_scope()->values.back(), 1};

    case schemas::OperatorEnum::LESS:
      if (!CheckValueTypes(ParseInfixOperatorOperands(operator_expression, path), {number_type, number_type}, path)) {
        return {};
      }
      InsertInstructions(path, { Instruction::CreateIsNumberLess() });
      current_scope()->PopValue();
      current_scope()->values.back().type_id = boolean_type;
      assert(!current_scope()->values.back().name.has_value());
      return {&current_scope()->values.back(), 1};

  }
}

std::span<ScriptFunctionScopeValue> ScriptFunctionParser::ParseInfixOperatorOperands(const schemas::OperatorClass& operator_expression, std::string_view path) {
  if (!operator_expression.left_hand_side || !operator_expression.right_hand_side) {
    AddError(path, "Operator requires a left and right hand side operand.");
    return {};
  }
  const auto lhs_path = fmt::format("{}/leftHandSide", path);
  auto lhs = ParseExpression(*operator_expression.left_hand_side, lhs_path);
  if (lhs.size() != 1) {
    AddError(lhs_path, "Left hand side expression must produce exacly one value.");
    return {};
  }
  const auto rhs_path = fmt::format("{}/rightHandSide", path);
  auto rhs = ParseExpression(*operator_expression.right_hand_side, rhs_path);
  if (rhs.size() != 1) {
    AddError(rhs_path, "Right hand side expression must produce exacly one value.");
    return {};
  }

  return {&current_scope()->values.back() - 1, 2};
}

bool ScriptFunctionParser::CheckValueTypes(std::span<ScriptFunctionScopeValue> values, const std::vector<TypeId>& expected_types, std::string_view path, std::string_view error_message) {
  bool correct = values.size() == expected_types.size();
  if (correct) {
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (values[i].type_id != expected_types[i]) {
        correct = false;
        break;
      }
    }
  }

  if (!correct) {
    std::string expected_string;
    if (expected_types.size() == 0) {
      expected_string = "None";
    } else if (expected_types.size() == 1) {
      expected_string = virtual_machine->GetType(expected_types[0])->name();
    } else if (expected_types.size() > 1) {
      expected_string += '[';
      for (const auto type_id : expected_types) {
        expected_string += virtual_machine->GetType(type_id)->name();
      }
      expected_string += ']';
    }

    std::string got_string;
    if (values.size() == 0) {
      got_string = "None";
    } else if (values.size() == 1) {
      got_string = virtual_machine->GetType(values[0].type_id)->name();
    } else if (values.size() > 1) {
      got_string += '[';
      for (const auto& value : values) {
        got_string += virtual_machine->GetType(value.type_id)->name();
      }
      got_string += ']';
    }
  
    AddError(path, "{} Expected {}, got {}", error_message.length() > 0 ? error_message : "Expression produced incorrect types.", expected_string, got_string);
  }

  return correct;
}

ScriptFunctionScope* ScriptFunctionParser::PushScope() {
  if (scopes.size() == 0) {
    scopes.push_back({
      .parent = nullptr,
      .base_index = 0,
    });
  } else {
    scopes.push_back({
      .parent = current_scope(),
      .base_index = static_cast<uint32_t>(current_scope()->base_index + current_scope()->values.size()),
    });
  }
  return &scopes.back();
}

void ScriptFunctionParser::PopScope() {
  // Last scope cannot be popped!
  assert(current_scope()->parent);
  InsertInstructions("", { Instruction::CreatePop(current_scope()->values.size()) });
  scopes.pop_back();
}

ScriptFunctionScope* ScriptFunctionParser::current_scope() {
  return &scopes.back();
}

template <typename T>
std::uint32_t ScriptFunctionParser::InsertConstant(T&& value) {
  const auto offset = definition.constants.size();
  definition.constants.push_back(virtual_machine->CreateValue(value));
  return offset;
}

template <typename T>
ScriptFunctionScopeValue* ScriptFunctionParser::InsertPushConstantInstructions(std::string_view path, T&& value) {
  const Type* type = virtual_machine->GetType<T>();
  if (!type->memory_layout().is_copyable) {
    AddError(path, "{} is not copyable", type->GetReferenceString());
    return nullptr;
  }
  
  const auto constant_index = InsertConstant(std::forward<T>(value));
  const auto temporary = current_scope()->PushValue(type->id());

  if (type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreatePushTrivialConstant(constant_index)
    });
  } else if (!type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreatePushAllocated(type->alignment_in_bytes(), type->size_in_bytes()),
      Instruction::CreatePushStackValueAllocatedAddress(temporary->index),
      Instruction::CreatePushConstantAllocatedAddress(constant_index),
      Instruction::CreateMemoryCopy(type->size_in_bytes()),
    });
  } else {
    InsertInstructions(path, {
      type->is_stored_inline()
        ? Instruction::CreatePush(1)
        : Instruction::CreatePushAllocated(type->alignment_in_bytes(), type->alignment_in_bytes())
    });
    InsertPrepareFunctionCallInstructions(path, type->copy_function());
    if (type->is_stored_inline()) {
      InsertInstructions(path, {
        Instruction::CreatePushStackValueDataAddress(temporary->index),
        Instruction::CreatePushConstantDataAddress(constant_index),
      });
    } else {
      InsertInstructions(path, {
        Instruction::CreatePushStackValueAllocatedAddress(temporary->index),
        Instruction::CreatePushConstantAllocatedAddress(constant_index),
      });
    }
    InsertFunctionCallInstructions(path, type->copy_function());
  }
  assert(current_scope()->current_stack_offset() == temporary->index + 1);

  return temporary;
}

ScriptFunctionScopeValue* ScriptFunctionParser::InsertConstructTypeInstructions(std::string_view path, NotNull<const Type*> type) {
  if (!type->memory_layout().is_constructible) {
    AddError(path, "{} is not constructible", type->GetReferenceString());
    return nullptr;
  }
  const auto construct_function = type->construct_function();
  assert(construct_function);

  auto value = current_scope()->PushValue(type->id());

  InsertInstructions(path, {
    type->is_stored_inline()
      ? Instruction::CreatePush(1)
      : Instruction::CreatePushAllocated(type->alignment_in_bytes(), type->alignment_in_bytes())
  });
  InsertPrepareFunctionCallInstructions(path, construct_function);
  InsertInstructions(path, {
    type->is_stored_inline()
      ? Instruction::CreatePushStackValueDataAddress(value->index)
      : Instruction::CreatePushStackValueAllocatedAddress(value->index),
  });
  current_scope()->PushValue(virtual_machine->GetTypeId<void*>());
  InsertFunctionCallInstructions(path, construct_function);

  return value;
}

void ScriptFunctionParser::InsertPopValueInstructions(std::string_view path, std::size_t count) {
  for (std::size_t i = 0; i < count; ++i) {
    Type* type = virtual_machine->GetType(current_scope()->values.back().type_id);
    if (type->trivially_destructible()) {
      InsertInstructions(path, {Instruction::CreatePopTrivial(1)});
    } else {
      InsertInstructions(path, {Instruction::CreatePop(1)});
    }
    current_scope()->PopValue();
  }
}

void ScriptFunctionParser::InsertCopyInstructions(std::string_view path, NotNull<const Type*> type, uint32_t destination_index, uint32_t source_index) {
  // assert(current_scope()->GetValue(destination_index)->type_id == type->id());
  // assert(current_scope()->GetValue(source_index)->type_id == type->id());

  if (!type->memory_layout().is_copyable) {
    AddError(path, "{} is not copyable", type->GetReferenceString());
    return;
  }
  
  if (type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreateCopyTrivial(destination_index, source_index),
    });
  } else if (!type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreatePushStackValueAllocatedAddress(destination_index),
      Instruction::CreatePushStackValueAllocatedAddress(source_index),
      Instruction::CreateMemoryCopy(type->size_in_bytes()),
    });
  } else {
    InsertPrepareFunctionCallInstructions(path, type->copy_function());
    if (type->is_stored_inline()) {
      InsertInstructions(path, {
        Instruction::CreatePushStackValueDataAddress(destination_index),
        Instruction::CreatePushStackValueDataAddress(source_index),
      });
    } else {
      InsertInstructions(path, {
        Instruction::CreatePushStackValueAllocatedAddress(destination_index),
        Instruction::CreatePushStackValueAllocatedAddress(source_index),
      });
    }
    InsertFunctionCallInstructions(path, type->copy_function());
  }
}

void ScriptFunctionParser::InsertAssignInstructions(std::string_view path, NotNull<const Type*> type, uint32_t destination_index) {
  if (type->is_stored_inline() && type->trivially_copyable()) {
    InsertInstructions(path, {
      Instruction::CreateAssignTrivial(destination_index),
    });
  } else {
    const auto source_index = current_scope()->values.back().index;
    InsertCopyInstructions(path, type, destination_index, source_index);
    InsertInstructions(path, {
      Instruction::CreatePop(1),
    });
  }
}

void ScriptFunctionParser::InsertPrepareFunctionCallInstructions(std::string_view path, NotNull<const Function*> function) {
  if (function->is_script_function()) {
    for (const auto& output : function->outputs()) {
      InsertConstructTypeInstructions(path, virtual_machine->GetType(output.type));
    }
    InsertInstructions(path, {
      Instruction::CreatePrepareScriptFunctionCall(function->outputs().size()),
    });
    current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
    current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
    current_scope()->PushValue(virtual_machine->GetTypeId<uint32_t>());
  }
}

void ScriptFunctionParser::InsertFunctionCallInstructions(std::string_view path, NotNull<const Function*> function) {
  if (function->is_script_function()) {
    InsertPushConstantInstructions(path, function->handle().instruction_offset);
    InsertInstructions(path, {
      Instruction::CreateScriptFunctionCall(function->outputs().size(), function->inputs().size()),
    });
    current_scope()->PopValue(); // function handle
    for (const auto& input : function->inputs()) {
      current_scope()->PopValue();
    }
    current_scope()->PopValue(); // return address
    current_scope()->PopValue(); // stack offset
    current_scope()->PopValue(); // constant offset
  } else {
    InsertPushConstantInstructions(path, function->handle());
    InsertInstructions(path, {
      Instruction::CreateCallNativeFunction(function->inputs().size()),
    });
    current_scope()->PopValue(); // function handle
    for (const auto& input : function->inputs()) {
      current_scope()->PopValue();
    }
    for (const auto& output : function->outputs()) {
      current_scope()->PushValue(output.type);
    }
  }
}

void ScriptFunctionParser::InsertInstructions(std::string_view path, std::initializer_list<Instruction> instructions) {
  definition.instructions.insert(definition.instructions.end(), instructions);
}

}  // namespace ovis
