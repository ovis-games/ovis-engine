#include <catch2/catch.hpp>

#include <ovis/core/scene_object.hpp>

using namespace ovis;

TEST_CASE("Parse object name", "[ovis][core][SceneObject]") {
  SECTION("object name without number") {
    auto result = SceneObject::ParseName("Test name");
    REQUIRE(result.first == "Test name");
    REQUIRE(!result.second.has_value());
  }

  SECTION("object name with number at the end") {
    auto result = SceneObject::ParseName("Test name123");
    REQUIRE(result.first == "Test name");
    REQUIRE(result.second.has_value());
    REQUIRE(result.second == 123);
  }

  SECTION("object name with number in the midle") {
    auto result = SceneObject::ParseName("Test 123 name");
    REQUIRE(result.first == "Test 123 name");
    REQUIRE(!result.second.has_value());
  }

  SECTION("object name with number in the middle and at the end") {
    auto result = SceneObject::ParseName("Test 234 name123");
    REQUIRE(result.first == "Test 234 name");
    REQUIRE(result.second.has_value());
    REQUIRE(result.second == 123);
  }
}
