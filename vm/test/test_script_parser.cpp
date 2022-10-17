#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include "ovis/vm/script_parser.hpp"

using namespace ovis;

const auto TEST_SCRIPT = R"(
[
  {
    "definitionType": "type",
    "name": "SomeType",
    "properties": [
      {
        "variableName": "SomeBoolean",
        "variableType": "Boolean"
      },
      {
        "variableName": "SomeNumber",
        "variableType": "Number"
      },
      {
        "variableName": "SomeTest",
        "variableType": "String"
      }
    ]
  },
  {
    "definitionType": "function",
    "name": "addNumbers",
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
]
)"_json;

TEST_CASE("Test script parsing", "[ovis][vm]") {
  VirtualMachine vm;
  auto test_module = vm.RegisterModule("Test");

  auto parse_script_result = ParseScript(&vm, TEST_SCRIPT);
  REQUIRE_RESULT(parse_script_result);
}

TEST_CASE("Test script parser", "[ovis][vm]") {
  VirtualMachine vm;
  ScriptParser parser(&vm, "TestModule");

  parser.AddScript(TEST_SCRIPT, "test");
  REQUIRE(parser.Parse());
}
