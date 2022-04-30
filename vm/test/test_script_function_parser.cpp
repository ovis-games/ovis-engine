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
      "actions": [
        {
          "id": "variable",
          "type": "Number"
        }
      ]
    }
    )"_json);
    REQUIRE_RESULT(parse_result);

    // REQUIRE(parse_result->instructions.size() == 3);
    // REQUIRE(parse_result->instructions[0].opcode == OpCode::PUSH);
    // REQUIRE(parse_result->instructions[0].push_pop.count == 1);
  }
}
