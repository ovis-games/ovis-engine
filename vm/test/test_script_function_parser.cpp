#include "test_utils.hpp"
#include <iostream>

#include <catch2/catch.hpp>

#include "ovis/utils/log.hpp"
#include "ovis/vm/script_function_parser.hpp"
#include "ovis/vm/virtual_machine.hpp"

using namespace ovis;

TEST_CASE("Script function parsing", "[ovis][core][ScriptFunctionParser]") {
  VirtualMachine vm;
  
  SECTION("Variable declaration") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [
        {
          "variableType": "Number",
          "variableName": "first"
        },
        {
          "variableType": "Number",
          "variableName": "second"
        }
      ],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "statementType": "variable_declaration",
          "variableName": "test",
          "variableType": "Number",
          "value": {
            "expressionType": "number_operation",
            "operation": "add",
            "firstOperand": {
              "expressionType": "variable",
              "name": "first"
            },
            "secondOperand": {
              "expressionType": "variable",
              "name": "second"
            }
          }
        },
        {
          "statementType": "return",
          "outputs": {
            "outputNumber": {
              "expressionType": "variable",
              "name": "test"
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE(parse_result.errors.empty());
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 2);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double(double, double)>(Function::Create(parse_result.function_description));
    const auto call_result = function(1.0, 2.0);
    REQUIRE_RESULT(call_result);
    // REQUIRE(*call_result == 3.0);
  }
}
