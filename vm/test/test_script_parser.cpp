#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include "ovis/vm/script_parser.hpp"

using namespace ovis;

TEST_CASE("Test script parsing", "[ovis][vm]") {
  VirtualMachine vm;
  auto test_module = vm.RegisterModule("Test");

  auto parse_script_result = ParseScript(&vm,  R"(
  [
    {
      "definitionType": "type",
      "name": "SomeType",
      "properties" : {
        "SomeBoolean": {
          "type": "Boolean"
        },
        "SomeNumber": {
          "type": "Number"
        },
        "SomeTest": {
          "type": "String"
        }
      }
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
          "type": "variable_declaration",
          "variable": {
            "name": "test",
            "type": "Number",
            "value": {
              "type": "number_operation",
              "operation": "add",
              "firstOperand": {
                "type": "variable",
                "name": "first"
              },
              "secondOperand": {
                "type": "variable",
                "name": "second"
              }

            }
          }
        },
        {
          "type": "return",
          "outputs": {
            "outputNumber": {
              "type": "variable",
              "name": "test"
            }
          }
        }
      ]
    }
  ]
  )"_json);
  REQUIRE_RESULT(parse_script_result);
}
