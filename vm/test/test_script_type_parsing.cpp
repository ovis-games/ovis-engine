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

TEST_CASE("Parse parse variable declaration", "[ovis][core][ScriptTypeParser]") {
  const auto parse_result = ParseScriptType(R"(
  {
    "name": "SomeType",
    "properties" : {
      "SomeBoolean": {
        "type": "Core.Boolean"
      },
      "SomeNumber": {
        "type": "Core.Number"
      }
    }
  }
  )"_json);
  REQUIRE_RESULT(parse_result);

  REQUIRE(parse_result->type != nullptr);
  REQUIRE(parse_result->type->name() == "SomeType");
  REQUIRE(parse_result->type->alignment_in_bytes() == 8);
  REQUIRE(parse_result->type->size_in_bytes() == 16);
}
