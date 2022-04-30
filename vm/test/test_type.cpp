#include <catch2/catch.hpp>

#include <ovis/vm/module.hpp>
#include <ovis/vm/type.hpp>
#include <ovis/vm/virtual_machine.hpp>

using namespace ovis;

TEST_CASE("Test built-in types", "[ovis][vm]") {
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

TEST_CASE("Test type registration", "[ovis][vm]") {
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
    REQUIRE(some_type->module() == nullptr);
    REQUIRE(some_type->base_id() == Type::NONE_ID);
    REQUIRE(some_type->size_in_bytes() == sizeof(SomeType));
    REQUIRE(some_type->alignment_in_bytes() == alignof(SomeType));
    REQUIRE(some_type->trivially_copyable());
    REQUIRE(some_type->trivially_destructible());

    REQUIRE(vm.RegisterType(TypeDescription::CreateForNativeType<SomeType, SomeBase>(&vm, "SomeType", test_module.get())) == some_type);
  }
}

// using SharedNumber = std::shared_ptr<double>;
// struct NonTrivial {
//   SharedNumber number;
// };

// TEST_CASE("Register non-trivial type", "[ovis][core][Type]") {
//   auto test_module = RegisterTestModule();
//   const auto non_trivial_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<NonTrivial>("NonTrivial"));
//   REQUIRE(non_trivial_type);

//   REQUIRE(non_trivial_type->size_in_bytes() == sizeof(NonTrivial));
//   REQUIRE(non_trivial_type->alignment_in_bytes() == alignof(NonTrivial));
//   REQUIRE(!non_trivial_type->trivially_copyable());
//   REQUIRE(!non_trivial_type->trivially_destructible());
// }

// struct Trivial {
//   double number;
// };

// TEST_CASE("Register trivial type", "[ovis][core][Type]") {
//   auto test_module = RegisterTestModule();
//   const auto trivial_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<Trivial>("Trivial"));
//   REQUIRE(trivial_type);

//   REQUIRE(trivial_type->size_in_bytes() == sizeof(Trivial));
//   REQUIRE(trivial_type->alignment_in_bytes() == alignof(Trivial));
//   REQUIRE(trivial_type->trivially_copyable());
//   REQUIRE(trivial_type->trivially_destructible());
// }

// TEST_CASE("Register trivial properties", "[ovis][core][Type]") {
//   auto test_module = RegisterTestModule();
//   auto trivial_type_description = ovis::TypeDescription::CreateForNativeType<Trivial>("Trivial");

//   trivial_type_description.properties.push_back(ovis::TypePropertyDescription::Create<&Trivial::number>("number"));
//   REQUIRE(trivial_type_description.properties[0].type == ovis::Type::GetId<double>());

//   const auto trivial_type = test_module->RegisterType(trivial_type_description);
//   REQUIRE(trivial_type);

//   auto value = ovis::Value::Create(Trivial { .number = 8.0 });
//   REQUIRE(value.as<Trivial>().number == 8.0);
//   value.SetProperty("number", 9.0);
//   REQUIRE(value.as<Trivial>().number == 9.0);
// }

// TEST_CASE("Register non-trivial properties", "[ovis][core][Type]") {
//   auto test_module = RegisterTestModule();
//   auto number_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<SharedNumber>("SharedNumber"));
//   auto non_trivial_type_description = ovis::TypeDescription::CreateForNativeType<NonTrivial>("NonTrivial");

//   non_trivial_type_description.properties.push_back(ovis::TypePropertyDescription::Create<&NonTrivial::number>("number"));
//   REQUIRE(non_trivial_type_description.properties[0].type == number_type->id());

//   const auto non_trivial_type = test_module->RegisterType(non_trivial_type_description);
//   REQUIRE(non_trivial_type);

//   auto value = ovis::Value::Create(NonTrivial { .number = std::make_shared<double>(8.0) });
//   REQUIRE(*value.as<NonTrivial>().number == 8.0);
//   value.SetProperty("number", std::make_shared<double>(9.0));
//   REQUIRE(*value.as<NonTrivial>().number == 9.0);
// }

// struct Base {
//   int x;
// };
// struct Derived : public Base {
//   virtual int foo() { return 0; }
// };
// struct Derived2 : public Derived {
//   virtual int foo() override { return 1; }
// };
// struct Unrelated {};

// TEST_CASE("Register type with base type", "[ovis][core][Type]") {
//   auto test_module = RegisterTestModule();
//   auto base_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<Base>("Base"));
//   auto derived_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<Derived, Base>("Derived"));
//   auto derived2_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<Derived2, Derived>("Derived2"));
//   auto unrelated_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<Unrelated>("Unrelated"));

//   Base base;

//   Derived derived;
//   Base* derived_base = &derived;

//   Derived2 derived2;
//   Derived* derived2_derived = &derived2;
//   Base* derived2_base = &derived2;

//   Unrelated unrelated;

//   REQUIRE(base_type->IsDerivedFrom<Base>());
//   REQUIRE(!base_type->IsDerivedFrom(derived_type->id()));
//   REQUIRE(!base_type->IsDerivedFrom(derived2_type->id()));
//   REQUIRE(!base_type->IsDerivedFrom(unrelated_type->id()));

//   REQUIRE(derived_type->IsDerivedFrom(base_type->id()));
//   REQUIRE(derived_type->IsDerivedFrom(derived_type->id()));
//   REQUIRE(!derived_type->IsDerivedFrom(derived2_type->id()));
//   REQUIRE(!derived_type->IsDerivedFrom(unrelated_type->id()));

//   REQUIRE(derived2_type->IsDerivedFrom(base_type->id()));
//   REQUIRE(derived2_type->IsDerivedFrom(derived_type->id()));
//   REQUIRE(derived2_type->IsDerivedFrom(derived2_type->id()));
//   REQUIRE(!derived2_type->IsDerivedFrom(unrelated_type->id()));

//   REQUIRE(!unrelated_type->IsDerivedFrom(base_type->id()));
//   REQUIRE(!unrelated_type->IsDerivedFrom(derived_type->id()));
//   REQUIRE(!unrelated_type->IsDerivedFrom(derived2_type->id()));
//   REQUIRE(unrelated_type->IsDerivedFrom(unrelated_type->id()));

//   REQUIRE(derived_type->CastToBase<Base>(&derived) == derived_base);
//   REQUIRE(derived2_type->CastToBase<Derived>(&derived2) == derived2_derived);
//   REQUIRE(derived2_type->CastToBase<Base>(&derived2) == derived2_base);
// }
