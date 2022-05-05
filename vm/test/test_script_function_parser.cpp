#include <iostream>

#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/vm/script_function_parser.hpp>

using namespace ovis;

#define REQUIRE_RESULT(expr) \
  do { \
    auto&& require_result = expr; \
    if (!require_result) { \
      for (const auto& error : require_result.error()) { \
        UNSCOPED_INFO(fmt::format("{}: {}", error.path.value_or(""), error.message)); \
      } \
    } \
    REQUIRE(require_result); \
  } while (false)

TEST_CASE("Script function parsing", "[ovis][core][ScriptFunctionParser]") {
  VirtualMachine vm;
  
  SECTION("Variable declaration") {
    const auto parse_result = ParseScriptFunction(&vm, R"(
    {
      "inputs": [
        {
          "type": "Number",
          "name": "some_number"
        },
        {
          "type": "Number",
          "name": "some_number3"
        }
      ],
      "outputs": [
        {
          "type": "Number",
          "name": "some_number2"
        }
      ],
      "actions": [
        {
          "id": "variable_declaration",
          "type": "Number"
        }
      ]
    }
    )"_json);
    REQUIRE_RESULT(parse_result);
    const FunctionDescription& function_description = parse_result->function_description;
    REQUIRE(function_description.inputs.size() == 2);
    REQUIRE(function_description.outputs.size() == 1);
  }
}
