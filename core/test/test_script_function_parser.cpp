#include <iostream>

#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_parser.hpp>

using namespace ovis;
using namespace ovis::vm;

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

TEST_CASE("Parse parse variable declaration", "[ovis][core][ScriptFunctionParser]") {
  const auto parse_result = ParseScriptFunction(R"(
  {
    "actions": [
      {
        "id": "variable",
        "type": "Core.Number"
      }
    ]
  }
  )"_json);
  REQUIRE_RESULT(parse_result);

  REQUIRE(parse_result->instructions.size() == 2);
  REQUIRE(parse_result->instructions[0].opcode == vm::OpCode::PUSH);
  REQUIRE(parse_result->instructions[0].push_pop.count == 1);
}
