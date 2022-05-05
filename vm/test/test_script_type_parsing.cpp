#include "test_utils.hpp"
#include <iostream>

#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/vm/script_type_parser.hpp>

using namespace ovis;

TEST_CASE("Parse parse variable declaration", "[ovis][core][ScriptTypeParser]") {
  VirtualMachine vm;
  auto test_module = vm.RegisterModule("Test");
  struct TestType {
    double x = 100.0;
  };
  auto test_type = vm.RegisterType<TestType>("Test", test_module.get());

  const auto parse_result = ParseScriptType(&vm, R"(
  {
    "name": "SomeType",
    "properties" : {
      "SomeBoolean": {
        "type": "Boolean"
      },
      "SomeNumber": {
        "type": "Number"
      },
      "SomeTest": {
        "type": "Test.Test"
      }
    }
  }
  )"_json);
  REQUIRE_RESULT(parse_result);

  REQUIRE(parse_result->type_description.name == "SomeType");
  REQUIRE(parse_result->type_description.memory_layout.alignment_in_bytes == 8);
  REQUIRE(parse_result->type_description.memory_layout.size_in_bytes == 24);
  REQUIRE(parse_result->type_description.properties.size() == 3);
  REQUIRE(parse_result->type_description.properties[0].name == "SomeBoolean");
  REQUIRE(parse_result->type_description.properties[0].type == vm.GetTypeId<bool>());
  REQUIRE(std::get<0>(parse_result->type_description.properties[0].access).offset == 0);
  REQUIRE(parse_result->type_description.properties[1].name == "SomeNumber");
  REQUIRE(parse_result->type_description.properties[1].type == vm.GetTypeId<double>());
  REQUIRE(std::get<0>(parse_result->type_description.properties[1].access).offset == 8);
  REQUIRE(parse_result->type_description.properties[2].name == "SomeTest");
  REQUIRE(parse_result->type_description.properties[2].type == test_type->id());
  REQUIRE(std::get<0>(parse_result->type_description.properties[2].access).offset == 16);

  const auto type = vm.RegisterType(parse_result->type_description);
  REQUIRE(type);

  Value value(type);

  {
    const auto some_boolean_result = value.GetProperty<bool>("SomeBoolean");
    REQUIRE_RESULT(some_boolean_result);
    REQUIRE(*some_boolean_result == false);

    const auto some_number_result = value.GetProperty<double>("SomeNumber");
    REQUIRE_RESULT(some_number_result);
    REQUIRE(*some_number_result == 0.0);

    const auto some_test_result = value.GetProperty<TestType>("SomeTest");
    REQUIRE_RESULT(some_test_result);
    REQUIRE(some_test_result->x == 100.0);
  }

  REQUIRE_RESULT(value.SetProperty("SomeBoolean", true));
  REQUIRE_RESULT(value.SetProperty("SomeNumber", 42.0));
  REQUIRE_RESULT(value.SetProperty("SomeTest", TestType{ .x = 123.0 }));

  {
    const auto some_boolean_result = value.GetProperty<bool>("SomeBoolean");
    REQUIRE_RESULT(some_boolean_result);
    REQUIRE(*some_boolean_result == true);

    const auto some_number_result = value.GetProperty<double>("SomeNumber");
    REQUIRE_RESULT(some_number_result);
    REQUIRE(*some_number_result == 42.0);

    const auto some_test_result = value.GetProperty<TestType>("SomeTest");
    REQUIRE_RESULT(some_test_result);
    REQUIRE(some_test_result->x == 123.0);
  }
}
