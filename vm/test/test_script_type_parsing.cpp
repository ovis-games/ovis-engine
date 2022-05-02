#include "test_utils.hpp"
#include <iostream>

#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/vm/script_type_parser.hpp>

using namespace ovis;

TEST_CASE("Parse parse variable declaration", "[ovis][core][ScriptTypeParser]") {
  VirtualMachine vm;

  const auto parse_result = ParseScriptType(&vm, R"(
  {
    "name": "SomeType",
    "properties" : {
      "SomeBoolean": {
        "type": "Boolean"
      },
      "SomeNumber": {
        "type": "Number"
      }
    }
  }
  )"_json);
  REQUIRE_RESULT(parse_result);

  REQUIRE(parse_result->type_description.name == "SomeType");
  REQUIRE(parse_result->type_description.memory_layout.alignment_in_bytes == 8);
  REQUIRE(parse_result->type_description.memory_layout.size_in_bytes == 16);
}
