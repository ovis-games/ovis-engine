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
  REQUIRE(parse_result->type_description.properties.size() == 2);
  REQUIRE(parse_result->type_description.properties[0].name == "SomeBoolean");
  REQUIRE(parse_result->type_description.properties[0].type == vm.GetTypeId<bool>());
  // REQUIRE(parse_result->type_description.properties[0].access == TypePropertyDescription::PrimitiveAccess{ .offset = 0 });
  REQUIRE(parse_result->type_description.properties[1].name == "SomeNumber");
  REQUIRE(parse_result->type_description.properties[1].type == vm.GetTypeId<double>());

  const auto type = vm.RegisterType(parse_result->type_description);
  REQUIRE(type);

  Value value(type);
  REQUIRE(value.GetProperty<double>("SomeNumber") == 0.0);
}
