#include "test_utils.hpp"

#include <catch2/catch.hpp>

#include "ovis/vm/value.hpp"
#include "ovis/vm/virtual_machine.hpp"

using namespace ovis;

TEST_CASE("Value", "[ovis][vm][Value]") {
  VirtualMachine vm;
  SECTION("Construct trivial value") {
    ovis::Value value = vm.CreateValue(8.0);
    REQUIRE(value.type() == vm.GetType<double>());
    REQUIRE(value.as<double>() == 8.0);
  }

  using SharedDouble = std::shared_ptr<double>;

  SECTION("Construct non-trivial value") {
    auto shared_double = std::make_shared<double>(8.0);
    REQUIRE(shared_double.use_count() == 1);
    {
      Value value = vm.CreateValue(shared_double);
      REQUIRE(value.type() == vm.GetType<SharedDouble>());
      REQUIRE(*value.as<SharedDouble>().get() == 8.0);
      REQUIRE(shared_double.use_count() == 2);
    }
    REQUIRE(shared_double.use_count() == 1);
  }

  SECTION("Copy non-trivial value") {
    auto shared_double = std::make_shared<double>(8.0);
    REQUIRE(shared_double.use_count() == 1);
    {
      ovis::Value value = vm.CreateValue(shared_double);
      REQUIRE(value.type() == vm.GetType<SharedDouble>());
      REQUIRE(*value.as<SharedDouble>().get() == 8.0);
      REQUIRE(shared_double.use_count() == 2);
      {
        ovis::Value value_copy(value);
        REQUIRE(value_copy.type() == vm.GetType<SharedDouble>());
        REQUIRE(*value_copy.as<SharedDouble>().get() == 8.0);
        REQUIRE(shared_double.use_count() == 3);
      }
      REQUIRE(shared_double.use_count() == 2);
    }
    REQUIRE(shared_double.use_count() == 1);
  }

  SECTION("Copy assign non-trivial value") {
    auto shared_double = std::make_shared<double>(8.0);
    auto other_shared_double = std::make_shared<double>(16.0);
    REQUIRE(shared_double.use_count() == 1);
    {
      ovis::Value value = vm.CreateValue(shared_double);
      REQUIRE(value.type() == vm.GetType<SharedDouble>());
      REQUIRE(*value.as<SharedDouble>().get() == 8.0);
      REQUIRE(shared_double.use_count() == 2);

      ovis::Value other_value = vm.CreateValue(other_shared_double);
      REQUIRE(other_value.type() == vm.GetType<SharedDouble>());
      REQUIRE(*other_value.as<SharedDouble>().get() == 16.0);
      REQUIRE(other_shared_double.use_count() == 2);

      other_value = value;

      REQUIRE(other_value.type() == vm.GetType<SharedDouble>());
      REQUIRE(*other_value.as<SharedDouble>().get() == 8.0);

      REQUIRE(value.type() == vm.GetType<SharedDouble>());
      REQUIRE(*value.as<SharedDouble>().get() == 8.0);

      REQUIRE(shared_double.use_count() == 3);
      REQUIRE(other_shared_double.use_count() == 1);
    }
    REQUIRE(shared_double.use_count() == 1);
    REQUIRE(other_shared_double.use_count() == 1);
  }

  SECTION("Copy to storage") {
    auto shared_double = std::make_shared<double>(8.0);
    REQUIRE(shared_double.use_count() == 1);

    Value value = vm.CreateValue(shared_double);
    REQUIRE(shared_double.use_count() == 2);

    ValueStorage storage;
    value.CopyTo(&storage);
    REQUIRE(shared_double.use_count() == 3);
    REQUIRE(storage.as<SharedDouble>() == shared_double);

    value.Reset();
    REQUIRE(shared_double.use_count() == 2);

    storage.Reset(vm.main_execution_context());
    REQUIRE(shared_double.use_count() == 1);
  }

  SECTION("Construct trivial type") {
    auto number_type = vm.GetType<double>();
    ovis::Value number_value(number_type);
    REQUIRE(number_value.type() == vm.GetType<double>());
    REQUIRE(number_value.as<double>() == 0.0);
  }

  SECTION("Construct type") {
    auto shared_number_type = vm.GetType<SharedDouble>();
    ovis::Value shared_number_value(shared_number_type);
    REQUIRE(shared_number_value.type() == vm.GetType<std::shared_ptr<double>>());
    REQUIRE(shared_number_value.as<std::shared_ptr<double>>() == nullptr);

    auto shared_number = std::make_shared<double>(8.0);
    REQUIRE(shared_number.use_count() == 1);
    shared_number_value.as<SharedDouble>() = shared_number;
    REQUIRE(shared_number.use_count() == 2);
    REQUIRE(shared_number_value.as<SharedDouble>() != nullptr);
    shared_number_value.Reset();
    REQUIRE(shared_number.use_count() == 1);
  }

  SECTION("Properties") {
    struct SomeType {
      bool some_bool = true;
      double some_number = 42.0;
    };
    auto type_desc = vm.CreateTypeDescription<SomeType>("SomeType", "Test");
    type_desc.properties.push_back(vm.CreateTypePropertyDescription<&SomeType::some_bool>("SomeBool"));
    type_desc.properties.push_back(vm.CreateTypePropertyDescription<&SomeType::some_number>("SomeNumber"));
    auto type = vm.RegisterType(type_desc);

    Value some_value(type);

    SomeType& some_type_reference = some_value.as<SomeType>();
    REQUIRE(some_type_reference.some_bool == true);
    REQUIRE(some_type_reference.some_number == 42.0);

    {
      const auto get_some_bool_result = some_value.GetProperty<bool>("SomeBool");
      REQUIRE_RESULT(get_some_bool_result);
      REQUIRE(*get_some_bool_result == true);

      const auto get_some_double_result = some_value.GetProperty<double>("SomeNumber");
      REQUIRE_RESULT(get_some_double_result);
      REQUIRE(*get_some_double_result == 42.0);
    }

    REQUIRE_RESULT(some_value.SetProperty("SomeBool", false));
    REQUIRE_RESULT(some_value.SetProperty("SomeNumber", 43.0));
    REQUIRE(some_type_reference.some_bool == false);
    REQUIRE(some_type_reference.some_number == 43.0);

    {
      const auto get_some_bool_result = some_value.GetProperty<bool>("SomeBool");
      REQUIRE_RESULT(get_some_bool_result);
      REQUIRE(*get_some_bool_result == false);

      const auto get_some_double_result = some_value.GetProperty<double>("SomeNumber");
      REQUIRE_RESULT(get_some_double_result);
      REQUIRE(*get_some_double_result == 43.0);
    }
  }
}



// Moving was removed
// TEST_CASE("Move non-trivial value", "[ovis][core][Value]") {
//   auto test_module = RegisterValueTestModule();
//   auto shared_double = std::make_shared<double>(8.0);
//   REQUIRE(shared_double.use_count() == 1);
//   {
//     ovis::Value value = Value::Create(ovis::Value::Create(vm, shared_double);
//     REQUIRE(value.type() == vm.GetType<SharedDouble>());
//     REQUIRE(*value.as<SharedDouble>().get() == 8.0);
//     REQUIRE(shared_double.use_count() == 2);
//     {
//       ovis::Value value_copy(std::move(value));
//       REQUIRE(value.as<SharedDouble>() == nullptr);
//       REQUIRE(shared_double.use_count() == 2);

//       REQUIRE(value_copy.type() == vm.GetType<SharedDouble>());
//       REQUIRE(*value_copy.as<SharedDouble>().get() == 8.0);
//     }
//     REQUIRE(shared_double.use_count() == 1);
//   }
//   REQUIRE(shared_double.use_count() == 1);
// }


// Moving was removed
// TEST_CASE("Move assign non-trivial value", "[ovis][core][Value]") {
//   auto test_module = RegisterValueTestModule();
//   auto shared_double = std::make_shared<double>(8.0);
//   auto other_shared_double = std::make_shared<double>(16.0);
//   REQUIRE(shared_double.use_count() == 1);
//   {
//     ovis::Value value = Value::Create(ovis::Value::Create(vm, shared_double);
//     REQUIRE(value.type() == vm.GetType<SharedDouble>());
//     REQUIRE(*value.as<SharedDouble>().get() == 8.0);
//     REQUIRE(shared_double.use_count() == 2);

//     ovis::Value other_value = Value::Create(ovis::Value::Create(vm, other_shared_double);
//     REQUIRE(other_value.type() == vm.GetType<SharedDouble>());
//     REQUIRE(*other_value.as<SharedDouble>().get() == 16.0);
//     REQUIRE(other_shared_double.use_count() == 2);

//     other_value = std::move(value);

//     REQUIRE(other_value.type() == vm.GetType<SharedDouble>());
//     REQUIRE(*other_value.as<SharedDouble>().get() == 8.0);

//     REQUIRE(value.type() == vm.GetType<SharedDouble>());
//     REQUIRE(value.as<SharedDouble>() == nullptr);

//     REQUIRE(shared_double.use_count() == 2);
//     REQUIRE(other_shared_double.use_count() == 1);
//   }
//   REQUIRE(shared_double.use_count() == 1);
//   REQUIRE(other_shared_double.use_count() == 1);
// }

