#include "catch2/catch_test_macros.hpp"

#include "ovis/vm/script_parser.hpp"
#include "ovis/test/require_result.hpp"

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
        "type": "Number",
        "name": "first"
      },
      {
        "type": "Number",
        "name": "second"
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
        "type": "variableDeclaration",
        "variableDeclaration": {
          "variable": {
            "name": "test",
            "type": "Number"
          },
          "value": {
            "type": "operator",
            "operator": {
              "operator": "add",
              "leftHandSide": {
                "type": "variable",
                "variable": "first"
              },
              "rightHandSide": {
                "type": "variable",
                "variable": "second"
              }
            }
          }
        }
      },
      {
        "type": "return",
        "return": {
          "type": "variable",
          "variable": "test"
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
