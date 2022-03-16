#include <catch2/catch.hpp>

#include "test_utils.hpp"

struct NonTrivial {
  std::shared_ptr<double> number;
};

TEST_CASE("Register non-trivial type", "[ovis][core][Type]") {
  auto test_module = RegisterTestModule();
  const auto non_trivial_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<NonTrivial>("NonTrivial"));
  REQUIRE(non_trivial_type);

  REQUIRE(non_trivial_type->size_in_bytes() == sizeof(NonTrivial));
  REQUIRE(non_trivial_type->alignment_in_bytes() == alignof(NonTrivial));
  REQUIRE(!non_trivial_type->trivially_copy_constructible());
  REQUIRE(!non_trivial_type->trivially_move_constructible());
  REQUIRE(!non_trivial_type->trivially_copy_assignable());
  REQUIRE(!non_trivial_type->trivially_move_assignable());
  REQUIRE(!non_trivial_type->trivially_destructible());
}

struct Trivial {
  double number;
};

TEST_CASE("Register trivial type", "[ovis][core][Type]") {
  auto test_module = RegisterTestModule();
  const auto trivial_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<Trivial>("Trivial"));
  REQUIRE(trivial_type);

  REQUIRE(trivial_type->size_in_bytes() == sizeof(Trivial));
  REQUIRE(trivial_type->alignment_in_bytes() == alignof(Trivial));
  REQUIRE(trivial_type->trivially_copy_constructible());
  REQUIRE(trivial_type->trivially_move_constructible());
  REQUIRE(trivial_type->trivially_copy_assignable());
  REQUIRE(trivial_type->trivially_move_assignable());
  REQUIRE(trivial_type->trivially_destructible());
}
