#include <iostream>

#include "catch2/catch_test_macros.hpp"

#include "ovis/utils/log.hpp"
#include "ovis/vm/script_function_parser.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/test/require_result.hpp"

#define REQUIRE_PARSE_RESULT(expr)                                                                  \
  do {                                                                                              \
    auto&& require_parse_result_value = expr;                                                       \
    if (!require_parse_result_value.errors.empty()) {                                               \
      for (const auto& error : require_parse_result_value.errors) {                                 \
        std::string location = "";                                                                  \
        if (error.location) {                                                                       \
          location = fmt::format("{}[{}]", error.location->script_name, error.location->json_path); \
        }                                                                                           \
        UNSCOPED_INFO(fmt::format("{}: {}", location, error.message));                              \
      }                                                                                             \
    }                                                                                               \
    REQUIRE(require_parse_result_value.errors.empty());                                             \
  } while (false)

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
          "return": {
            "type": "constant",
            "constant": 42.0
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
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
          "return": {
            "type": "operator",
            "operator": {
              "operator": "add",
              "leftHandSide": {
                "type": "constant",
                "constant": 40.5
              },
              "rightHandSide": {
                "type": "constant",
                "constant": 1.5
              }
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
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
          "return": {
            "type": "operator",
            "operator": {
              "operator": "greater",
              "leftHandSide": {
                "type": "constant",
                "constant": 40.5
              },
              "rightHandSide": {
                "type": "constant",
                "constant": 1.5
              }
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
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
          "return": {
            "type": "constant",
            "constant": 42.0
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
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
          "return": {
            "type": "variable",
            "variable": "test"
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
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

  SECTION("Function call expression with native function") {
    struct AddOne {
      static double Call(double input) { return input + 1.0; }
    };
    vm.RegisterFunction<&AddOne::Call>("addOne", "Test", { "input" }, { "output" });

    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "name": "callNativeFunction",
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
          "return": {
            "type": "functionCall",
            "functionCall": {
              "function": "Test.addOne",
              "inputs": [
                {
                  "type": "variable",
                  "variable": "test"
                }
              ]
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 1);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double(double)>(vm.RegisterFunction(parse_result.function_description));
    {
      const auto call_result = function(41);
      REQUIRE_RESULT(call_result);
      UNSCOPED_INFO(function_description.PrintDefinition());
      REQUIRE(*call_result == 42.0);
    }
    {
      const auto call_result = function(1336);
      REQUIRE_RESULT(call_result);
      REQUIRE(*call_result == 1337.0);
    }
  }

  SECTION("Function call expression with script function") {
    const auto add_one_parse_result = ParseScriptFunction(&vm, R"(
    {
      "name": "addOne",
      "inputs": [
        {
          "type": "Number",
          "name": "input"
        }
      ],
      "outputs": [
        {
          "type": "Number",
          "name": "output"
        }
      ],
      "statements": [
        {
          "type": "return",
          "return": {
            "type": "operator",
            "operator": {
              "operator": "add",
              "leftHandSide": {
                "type": "variable",
                "variable": "input"
              },
              "rightHandSide": {
                "type": "constant",
                "constant": 1
              }
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(add_one_parse_result);
    FunctionDescription add_one_function_desc = add_one_parse_result.function_description;
    add_one_function_desc.module = "Test";
    REQUIRE(add_one_function_desc.inputs.size() == 1);
    REQUIRE(add_one_function_desc.outputs.size() == 1);
    vm.RegisterFunction(add_one_function_desc);

    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "name": "callScriptFunction",
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
          "return": {
            "type": "functionCall",
            "functionCall": {
              "function": "Test.addOne",
              "inputs": [
                {
                  "type": "variable",
                  "variable": "test"
                }
              ]
            }
          }
        }
      ]
    }
    )"_json);
    REQUIRE_PARSE_RESULT(parse_result);
    const FunctionDescription& function_description = parse_result.function_description;
    REQUIRE(function_description.inputs.size() == 1);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = FunctionWrapper<double(double)>(vm.RegisterFunction(parse_result.function_description));
    {
      UNSCOPED_INFO(add_one_function_desc.PrintDefinition());
      UNSCOPED_INFO(function_description.PrintDefinition());
      const auto call_result = function(41);
      REQUIRE_RESULT(call_result);
      REQUIRE(*call_result == 42.0);
    }
    {
      const auto call_result = function(1336);
      REQUIRE_RESULT(call_result);
      REQUIRE(*call_result == 1337.0);
    }
  }
}
