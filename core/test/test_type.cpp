#include "test_utils.hpp"

#include <catch2/catch.hpp>

#include <ovis/core/value.hpp>

using SharedNumber = std::shared_ptr<double>;
struct NonTrivial {
  SharedNumber number;
};

TEST_CASE("Register non-trivial type", "[ovis][core][Type]") {
  auto test_module = RegisterTestModule();
  const auto non_trivial_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<NonTrivial>("NonTrivial"));
  REQUIRE(non_trivial_type);

  REQUIRE(non_trivial_type->size_in_bytes() == sizeof(NonTrivial));
  REQUIRE(non_trivial_type->alignment_in_bytes() == alignof(NonTrivial));
  REQUIRE(!non_trivial_type->trivially_copy_assignable());
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
  REQUIRE(trivial_type->trivially_copy_assignable());
  REQUIRE(trivial_type->trivially_destructible());
}

TEST_CASE("Register trivial properties", "[ovis][core][Type]") {
  auto test_module = RegisterTestModule();
  auto number_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<double>("Number"));
  auto trivial_type_description = ovis::TypeDescription::CreateForNativeType<Trivial>("Trivial");

  trivial_type_description.properties.push_back(ovis::TypePropertyDescription::Create<&Trivial::number>("number"));
  REQUIRE(trivial_type_description.properties[0].type == number_type);

  const auto trivial_type = test_module->RegisterType(trivial_type_description);
  REQUIRE(trivial_type);

  auto value = ovis::Value::Create(Trivial { .number = 8.0 });
  REQUIRE(value.as<Trivial>().number == 8.0);
  value.SetProperty("number", 9.0);
  REQUIRE(value.as<Trivial>().number == 9.0);
}

TEST_CASE("Register non-trivial properties", "[ovis][core][Type]") {
  auto test_module = RegisterTestModule();
  auto number_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<SharedNumber>("SharedNumber"));
  auto non_trivial_type_description = ovis::TypeDescription::CreateForNativeType<NonTrivial>("NonTrivial");

  non_trivial_type_description.properties.push_back(ovis::TypePropertyDescription::Create<&NonTrivial::number>("number"));
  REQUIRE(non_trivial_type_description.properties[0].type == number_type);

  const auto non_trivial_type = test_module->RegisterType(non_trivial_type_description);
  REQUIRE(non_trivial_type);

  auto value = ovis::Value::Create(NonTrivial { .number = std::make_shared<double>(8.0) });
  REQUIRE(*value.as<NonTrivial>().number == 8.0);
  value.SetProperty("number", std::make_shared<double>(9.0));
  REQUIRE(*value.as<NonTrivial>().number == 9.0);
}
