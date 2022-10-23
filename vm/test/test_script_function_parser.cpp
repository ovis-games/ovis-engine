#include <iostream>

#include "catch2/catch_test_macros.hpp"

#include "ovis/utils/log.hpp"
#include "ovis/vm/script_function_parser.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/test/require_result.hpp"

using namespace ovis;

TEST_CASE("Script function parsing", "[ovis][core][ScriptFunctionParser]") {
  VirtualMachine vm;

  SECTION("Return statement") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "statementType": "return",
          "outputs": {
            "outputNumber": 42.0
          }
        }
      ]
    }
    )"_json);
    REQUIRE(parse_result.errors.empty());
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 0);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double()>(Function::Create(parse_result.function_description));
    const auto call_result = function();
    REQUIRE_RESULT(call_result);
    REQUIRE(*call_result == 42.0);
  }

  SECTION("Add number operation expression") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "statementType": "return",
          "outputs": {
            "outputNumber": {
              "expressionType": "number_operation",
              "operation": "add",
              "firstOperand": 40.5,
              "secondOperand": 1.5
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE(parse_result.errors.empty());
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 0);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double()>(Function::Create(parse_result.function_description));
    const auto call_result = function();
    REQUIRE_RESULT(call_result);
    REQUIRE(*call_result == 42.0);
  }

  SECTION("Greater number operation expression") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "statementType": "return",
          "outputs": {
            "outputNumber": {
              "expressionType": "number_operation",
              "operation": "greater",
              "firstOperand": 40.5,
              "secondOperand": 1.5
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE(parse_result.errors.empty());
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 0);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<bool()>(Function::Create(parse_result.function_description));
    const auto call_result = function();
    REQUIRE_RESULT(call_result);
    REQUIRE(*call_result == true);
  }

  SECTION("Variable declaration statement") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "statementType": "variable_declaration",
          "variable": {
            "name": "test",
            "type": "Number"
          },
          "value": 1337.0
        },
        {
          "statementType": "return",
          "outputs": {
            "outputNumber": 42.0
          }
        }
      ]
    }
    )"_json);
    REQUIRE(parse_result.errors.empty());
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 0);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double()>(Function::Create(parse_result.function_description));
    const auto call_result = function();
    REQUIRE_RESULT(call_result);
    REQUIRE(*call_result == 42.0);
  }

  SECTION("Variable expression") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [
        {
          "type": "Number",
          "name": "someNumber"
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
          "statementType": "return",
          "outputs": {
            "outputNumber": {
              "expressionType": "variable",
              "name": "someNumber"
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE(parse_result.errors.empty());
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 1);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double(double)>(Function::Create(parse_result.function_description));
    {
      const auto call_result = function(42);
      REQUIRE_RESULT(call_result);
      REQUIRE(*call_result == 42.0);
    }
    {
      const auto call_result = function(1337);
      REQUIRE_RESULT(call_result);
      REQUIRE(*call_result == 1337.0);
    }
  }
  
  // SECTION("Variable declaration") {
  //   const auto parse_result = ParseScriptFunction(&vm, R"(
  //   {
  //     "inputs": [
  //       {
  //         "type": "Number",
  //         "name": "first"
  //       },
  //       {
  //         "type": "Number",
  //         "name": "second"
  //       }
  //     ],
  //     "outputs": [
  //       {
  //         "type": "Number",
  //         "name": "outputNumber"
  //       }
  //     ],
  //     "statements": [
  //       {
  //         "statementType": "variable_declaration",
  //         "name": "test",
  //         "type": "Number",
  //         "value": {
  //           "expressionType": "number_operation",
  //           "operation": "add",
  //           "firstOperand": {
  //             "expressionType": "variable",
  //             "name": "first"
  //           },
  //           "secondOperand": {
  //             "expressionType": "variable",
  //             "name": "second"
  //           }
  //         }
  //       },
  //       {
  //         "statementType": "return",
  //         "outputs": {
  //           "outputNumber": {
  //             "expressionType": "variable",
  //             "name": "test"
  //           }
  //         }
  //       }
  //     ]
  //   }
  //   )"_json);
  //   REQUIRE(parse_result.errors.empty());
  //   const FunctionDescription& function_description = parse_result.function_description;
  //   REQUIRE(function_description.inputs.size() == 2);
  //   REQUIRE(function_description.outputs.size() == 1);
  //   const auto function = FunctionWrapper<double(double, double)>(Function::Create(parse_result.function_description));
  //   const auto call_result = function(1.0, 2.0);
  //   REQUIRE_RESULT(call_result);
  //   // REQUIRE(*call_result == 3.0);
  // }
}
