#include <iostream>

#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/utils/log.hpp>
#include <ovis/vm/script_function_parser.hpp>

using namespace ovis;

TEST_CASE("Script function parsing", "[ovis][core][ScriptFunctionParser]") {
  VirtualMachine vm;
  
  SECTION("Variable declaration") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
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
    )"_json);
    REQUIRE_RESULT(parse_result);
    const FunctionDescription& function_description = parse_result->function_description;
    REQUIRE(function_description.inputs.size() == 2);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = Function::Create(parse_result->function_description);
    const auto call_result = function->Call<double>(1.0, 2.0);
    REQUIRE_RESULT(call_result);
    REQUIRE(*call_result == 3.0);
  }
}
