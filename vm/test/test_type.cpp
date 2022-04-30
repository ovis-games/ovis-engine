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
    REQUIRE(!some_type->IsDerivedFrom<SomeBase>());

    REQUIRE(vm.RegisterType(TypeDescription::CreateForNativeType<SomeType, SomeBase>(&vm, "SomeType", test_module.get())) == some_type);
    REQUIRE(some_type->id() == some_type_id);
    REQUIRE(some_type->name() == "SomeType");
    REQUIRE(some_type->module() == test_module.get());
    REQUIRE(some_type->base_id() == vm.GetTypeId<SomeBase>());
    REQUIRE(some_type->size_in_bytes() == sizeof(SomeType));
    REQUIRE(some_type->alignment_in_bytes() == alignof(SomeType));
    REQUIRE(some_type->trivially_copyable());
    REQUIRE(some_type->trivially_destructible());
    REQUIRE(some_type->IsDerivedFrom<SomeBase>());

    const Type* some_base_type = vm.GetType<SomeBase>();
    REQUIRE(some_base_type != nullptr);
    REQUIRE(some_base_type->id() != Type::NONE_ID);
    REQUIRE(some_base_type->id() != some_type_id);
    REQUIRE(some_base_type->name() == "");
    REQUIRE(some_base_type->module() == nullptr);
    REQUIRE(some_base_type->base_id() == Type::NONE_ID);
    REQUIRE(some_base_type->size_in_bytes() == sizeof(SomeBase));
    REQUIRE(some_base_type->alignment_in_bytes() == alignof(SomeBase));
    REQUIRE(some_base_type->trivially_copyable());
    REQUIRE(some_base_type->trivially_destructible());
  }

  using SharedNumber = std::shared_ptr<double>;
  struct NonTrivial {
    SharedNumber number;
  };
  SECTION("Non-trivial type registration") {
    const auto non_trivial_type = vm.RegisterType(TypeDescription::CreateForNativeType<NonTrivial>(&vm, "NonTrivial"));
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
    auto trivial_type_description = TypeDescription::CreateForNativeType<Trivial>(&vm, "Trivial");

    trivial_type_description.properties.push_back(TypePropertyDescription::Create<&Trivial::number>(&vm, "number"));
    REQUIRE(trivial_type_description.properties[0].type == vm.GetTypeId<double>());

    const auto trivial_type = vm.RegisterType(trivial_type_description);
    REQUIRE(trivial_type);

    auto value = Value::Create(&vm, Trivial { .number = 8.0 });
    REQUIRE(value.as<Trivial>().number == 8.0);
    value.SetProperty("number", 9.0);
    REQUIRE(value.as<Trivial>().number == 9.0);
  }

  SECTION("Non-trivial property registration") {
    auto non_trivial_type_description = TypeDescription::CreateForNativeType<NonTrivial>(&vm, "NonTrivial");

    non_trivial_type_description.properties.push_back(TypePropertyDescription::Create<&NonTrivial::number>(&vm, "number"));
    REQUIRE(non_trivial_type_description.properties[0].type == vm.GetTypeId<SharedNumber>());

    const auto non_trivial_type = vm.RegisterType(non_trivial_type_description);
    REQUIRE(non_trivial_type);

    auto value = Value::Create(&vm, NonTrivial { .number = std::make_shared<double>(8.0) });
    REQUIRE(*value.as<NonTrivial>().number == 8.0);
    value.SetProperty("number", std::make_shared<double>(9.0));
    REQUIRE(*value.as<NonTrivial>().number == 9.0);
  }

  SECTION("Register with base types") {
    struct Base {
      int x;
    };
    struct Derived : public Base {
      virtual int foo() { return 0; }
    };
    struct Derived2 : public Derived {
      virtual int foo() override { return 1; }
    };
    struct Unrelated {};
    auto base_type = vm.RegisterType(TypeDescription::CreateForNativeType<Base>(&vm, "Base"));
    auto derived_type = vm.RegisterType(TypeDescription::CreateForNativeType<Derived, Base>(&vm, "Derived"));
    auto derived2_type = vm.RegisterType(TypeDescription::CreateForNativeType<Derived2, Derived>(&vm, "Derived2"));
    auto unrelated_type = vm.RegisterType(TypeDescription::CreateForNativeType<Unrelated>(&vm, "Unrelated"));

    Base base;

    Derived derived;
    Base* derived_base = &derived;

    Derived2 derived2;
    Derived* derived2_derived = &derived2;
    Base* derived2_base = &derived2;

    Unrelated unrelated;

    REQUIRE(base_type->IsDerivedFrom<Base>());
    REQUIRE(!base_type->IsDerivedFrom(derived_type->id()));
    REQUIRE(!base_type->IsDerivedFrom(derived2_type->id()));
    REQUIRE(!base_type->IsDerivedFrom(unrelated_type->id()));

    REQUIRE(derived_type->IsDerivedFrom(base_type->id()));
    REQUIRE(derived_type->IsDerivedFrom(derived_type->id()));
    REQUIRE(!derived_type->IsDerivedFrom(derived2_type->id()));
    REQUIRE(!derived_type->IsDerivedFrom(unrelated_type->id()));

    REQUIRE(derived2_type->IsDerivedFrom(base_type->id()));
    REQUIRE(derived2_type->IsDerivedFrom(derived_type->id()));
    REQUIRE(derived2_type->IsDerivedFrom(derived2_type->id()));
    REQUIRE(!derived2_type->IsDerivedFrom(unrelated_type->id()));

    REQUIRE(!unrelated_type->IsDerivedFrom(base_type->id()));
    REQUIRE(!unrelated_type->IsDerivedFrom(derived_type->id()));
    REQUIRE(!unrelated_type->IsDerivedFrom(derived2_type->id()));
    REQUIRE(unrelated_type->IsDerivedFrom(unrelated_type->id()));

    REQUIRE(derived_type->CastToBase<Base>(&derived) == derived_base);
    REQUIRE(derived2_type->CastToBase<Derived>(&derived2) == derived2_derived);
    REQUIRE(derived2_type->CastToBase<Base>(&derived2) == derived2_base);
  }
}



