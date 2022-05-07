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
      "outputs": [
        {
          "type": "Number",
          "name": "outputNumber"
        }
      ],
      "actions": [
        {
          "id": "return",
          "outputs": {
            "outputNumber": 42.0
          }
        }
      ]
    }
    )"_json);
    REQUIRE_RESULT(parse_result);
    const FunctionDescription& function_description = parse_result->function_description;
    REQUIRE(function_description.inputs.size() == 0);
    REQUIRE(function_description.outputs.size() == 1);
    const auto function = Function::Create(parse_result->function_description);
    const auto call_result = function->Call<double>();
    REQUIRE_RESULT(call_result);
    REQUIRE(*call_result == 42.0);
  }
}
