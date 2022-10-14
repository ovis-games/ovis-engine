#include <catch2/catch.hpp>

#include "ovis/vm/type.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/virtual_machine.hpp"

using namespace ovis;

TEST_CASE("Test built-in types", "[ovis][vm][Type]") {
  VirtualMachine vm;

  SECTION("None type") {
    REQUIRE(vm.GetTypeId<void>() == Type::NONE_ID);

    Type* none_type = vm.GetType<void>();
    REQUIRE(none_type != nullptr);
    REQUIRE(none_type->name() == "None");
  }

  SECTION("Boolean type") {
    REQUIRE(vm.GetTypeId<bool>() != Type::NONE_ID);

    Type* boolean_type = vm.GetType<bool>();
    REQUIRE(boolean_type != nullptr);
    REQUIRE(boolean_type ->name() == "Boolean");
  }

  SECTION("Number type") {
    REQUIRE(vm.GetTypeId<double>() != Type::NONE_ID);

    Type* number_type = vm.GetType<double>();
    REQUIRE(number_type != nullptr);
    REQUIRE(number_type ->name() == "Number");
  }

  SECTION("String type") {
    REQUIRE(vm.GetTypeId<std::string>() != Type::NONE_ID);

    Type* string_type = vm.GetType<std::string>();
    REQUIRE(string_type != nullptr);
    REQUIRE(string_type ->name() == "String");
  }
}

TEST_CASE("Test type registration", "[ovis][vm][Type]") {
  VirtualMachine vm;
  auto test_module = vm.RegisterModule("Test");

  SECTION("On-the-fly typeid generation") {
    struct SomeBase {};
    struct SomeType : public SomeBase {};
    const auto some_type_id = vm.GetTypeId<SomeType>();
    REQUIRE(some_type_id != Type::NONE_ID);

    const Type* some_type = vm.GetType(some_type_id);
    REQUIRE(some_type != nullptr);
    REQUIRE(some_type->id() == some_type_id);
    REQUIRE(some_type->name() == "");
    REQUIRE(some_type->module() == "");
    REQUIRE(some_type->size_in_bytes() == sizeof(SomeType));
    REQUIRE(some_type->alignment_in_bytes() == alignof(SomeType));
    REQUIRE(some_type->trivially_copyable());
    REQUIRE(some_type->trivially_destructible());
  }

  using SharedNumber = std::shared_ptr<double>;
  struct NonTrivial {
    SharedNumber number;
  };
  SECTION("Non-trivial type registration") {
    const auto non_trivial_type = vm.RegisterType<NonTrivial>("NonTrivial", "Test");
    REQUIRE(non_trivial_type);

    REQUIRE(non_trivial_type->size_in_bytes() == sizeof(NonTrivial));
    REQUIRE(non_trivial_type->alignment_in_bytes() == alignof(NonTrivial));
    REQUIRE(!non_trivial_type->trivially_copyable());
    REQUIRE(!non_trivial_type->trivially_destructible());
  }

  SECTION("Trivial property registration") {
    struct Trivial {
      double number;
    };
    auto trivial_type_description = vm.CreateTypeDescription<Trivial>("Trivial", "vm");

    trivial_type_description.properties.push_back(vm.CreateTypePropertyDescription<&Trivial::number>("number"));
    REQUIRE(trivial_type_description.properties[0].type == vm.GetTypeId<double>());

    const auto trivial_type = vm.RegisterType(trivial_type_description);
    REQUIRE(trivial_type);

    auto value = Value::Create(&vm, Trivial { .number = 8.0 });
    REQUIRE(value.as<Trivial>().number == 8.0);
    value.SetProperty("number", 9.0);
    REQUIRE(value.as<Trivial>().number == 9.0);
  }

  SECTION("Non-trivial property registration") {
    auto non_trivial_type_description = vm.CreateTypeDescription<NonTrivial>("NonTrivial", "");

    non_trivial_type_description.properties.push_back(vm.CreateTypePropertyDescription<&NonTrivial::number>("number"));
    REQUIRE(non_trivial_type_description.properties[0].type == vm.GetTypeId<SharedNumber>());

    const auto non_trivial_type = vm.RegisterType(non_trivial_type_description);
    REQUIRE(non_trivial_type);

    auto value = Value::Create(&vm, NonTrivial { .number = std::make_shared<double>(8.0) });
    REQUIRE(*value.as<NonTrivial>().number == 8.0);
    value.SetProperty("number", std::make_shared<double>(9.0));
    REQUIRE(*value.as<NonTrivial>().number == 9.0);
  }
}
