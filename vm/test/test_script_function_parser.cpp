#include <iostream>

#include "catch2/catch_test_macros.hpp"

#include "ovis/utils/log.hpp"
#include "ovis/vm/script_function_parser.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/test/require_result.hpp"

using namespace ovis;

TEST_CASE("Script function parsing", "[ovis][core][ScriptFunctionParser]") {
  VirtualMachine vm;

// An empty function is actually invalid but we currently do not test for it
//   SECTION("Empty") {
//     const auto parse_result = ParseScriptFunction(&vm, R"(
//     {
//       "name": "empty",
//       "inputs": [],
//       "outputs": [],
//       "statements": []
//     }
//     )"_json);
//     REQUIRE(parse_result.errors.empty());
//     const FunctionDescription& function_description = parse_result.function_description;
//     REQUIRE(function_description.inputs.size() == 0);
//     REQUIRE(function_description.outputs.size() == 0);
//     const auto function = FunctionWrapper<void()>(Function::Create(parse_result.function_description));
//     const auto call_result = function();
//     REQUIRE_RESULT(call_result);
//   }

  SECTION("Return statement") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "name": "return",
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "type": "return",
          "return": [
            {
              "type": "constant",
              "constant": 42.0
            }
          ]
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

  SECTION("Add add operator expression") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "name": "operator",
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "type": "return",
          "return": [
            {
              "type": "operator",
              "operator": {
                "operator": "add",
                "operands": [
                  {
                    "type": "constant",
                    "constant": 40.5
                  },
                  {
                    "type": "constant",
                    "constant": 1.5
                  }
                ]
              }
            }
          ]
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
      "name": "operator",
      "inputs": [],
      "outputs": [
        {
          "type": "Boolean",
          "name": "isGreater"
        }
      ],
      "statements": [
        {
          "type": "return",
          "return": [
            {
              "type": "operator",
              "operator": {
                "operator": "greater",
                "operands": [
                  {
                    "type": "constant",
                    "constant": 40.5
                  },
                  {
                    "type": "constant",
                    "constant": 1.5
                  }
                ]
              }
            }
          ]
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
      "name": "varaibleDeclaration",
      "inputs": [],
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "statements": [
        {
          "type": "variableDeclaration",
          "variableDeclaration": {
            "variable": {
              "name": "test",
              "type": "Number"
            },
            "value": {
              "type": "constant",
              "constant": 1337.0
            }
          }
        },
        {
          "type": "return",
          "return": [
            {
              "type": "constant",
              "constant": 42.0
            }
          ]
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
      "name": "variableExpression",
      "inputs": [
        {
          "type": "Number",
          "name": "test"
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
          "type": "return",
          "return": [
            {
              "type": "variable",
              "variable": "test"
            }
          ]
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
      const auto call_result = function(43);
      REQUIRE_RESULT(call_result);
      UNSCOPED_INFO(function_description.PrintDefinition());
      REQUIRE(*call_result == 43.0);
    }
    {
      const auto call_result = function(1337);
      REQUIRE_RESULT(call_result);
      REQUIRE(*call_result == 1337.0);
    }
  }
}
