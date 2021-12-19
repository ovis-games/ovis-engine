#include <catch2/catch.hpp>

#include "test_utils.hpp"

struct TestType {
  std::shared_ptr<double> number;
};

TEST_CASE("Register type", "[ovis][core][Type]") {
  auto test_module = RegisterTestModule();
  auto test_type = test_module->RegisterType<TestType>("TestType");
  REQUIRE(test_type);

  REQUIRE(test_type->size_in_bytes() == sizeof(TestType));
  REQUIRE(test_type->alignment_in_bytes() == alignof(TestType));
  REQUIRE(test_type->copy_constructible());
  REQUIRE(!test_type->trivially_copy_constructible());
  REQUIRE(test_type->move_constructible());
  REQUIRE(!test_type->trivially_move_constructible());
  REQUIRE(test_type->copy_assignable());
  REQUIRE(!test_type->trivially_copy_assignable());
  REQUIRE(test_type->move_assignable());
  REQUIRE(!test_type->trivially_move_assignable());
}
