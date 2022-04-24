#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/core/value.hpp>

using SharedDouble = std::shared_ptr<double>;

std::shared_ptr<ovis::Module> RegisterValueTestModule() {
  auto module = RegisterTestModule();
  module->RegisterType(ovis::TypeDescription::CreateForNativeType<double>("Number"));
  module->RegisterType(ovis::TypeDescription::CreateForNativeType<SharedDouble>("SharedNumber"));
  return module;
}

TEST_CASE("Construct trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  ovis::Value value = ovis::Value::Create(8.0);
  REQUIRE(value.type() == ovis::Type::Get<double>());
  REQUIRE(value.as<double>() == 8.0);
}

TEST_CASE("Construct non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value = ovis::Value::Create(shared_double);
    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);
  }
  REQUIRE(shared_double.use_count() == 1);
}

TEST_CASE("Copy non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value = ovis::Value::Create(shared_double);
    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);
    {
      ovis::Value value_copy(value);
      REQUIRE(value_copy.type() == ovis::Type::Get<SharedDouble>());
      REQUIRE(*value_copy.as<SharedDouble>().get() == 8.0);
      REQUIRE(shared_double.use_count() == 3);
    }
    REQUIRE(shared_double.use_count() == 2);
  }
  REQUIRE(shared_double.use_count() == 1);
}

// Moving was removed
// TEST_CASE("Move non-trivial value", "[ovis][core][Value]") {
//   auto test_module = RegisterValueTestModule();
//   auto shared_double = std::make_shared<double>(8.0);
//   REQUIRE(shared_double.use_count() == 1);
//   {
//     ovis::Value value = ovis::Value::Create(shared_double);
//     REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
//     REQUIRE(*value.as<SharedDouble>().get() == 8.0);
//     REQUIRE(shared_double.use_count() == 2);
//     {
//       ovis::Value value_copy(std::move(value));
//       REQUIRE(value.as<SharedDouble>() == nullptr);
//       REQUIRE(shared_double.use_count() == 2);

//       REQUIRE(value_copy.type() == ovis::Type::Get<SharedDouble>());
//       REQUIRE(*value_copy.as<SharedDouble>().get() == 8.0);
//     }
//     REQUIRE(shared_double.use_count() == 1);
//   }
//   REQUIRE(shared_double.use_count() == 1);
// }

TEST_CASE("Copy assign non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  auto other_shared_double = std::make_shared<double>(16.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value = ovis::Value::Create(shared_double);
    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);

    ovis::Value other_value = ovis::Value::Create(other_shared_double);
    REQUIRE(other_value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*other_value.as<SharedDouble>().get() == 16.0);
    REQUIRE(other_shared_double.use_count() == 2);

    other_value = value;

    REQUIRE(other_value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*other_value.as<SharedDouble>().get() == 8.0);

    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);

    REQUIRE(shared_double.use_count() == 3);
    REQUIRE(other_shared_double.use_count() == 1);
  }
  REQUIRE(shared_double.use_count() == 1);
  REQUIRE(other_shared_double.use_count() == 1);
}

// Moving was removed
// TEST_CASE("Move assign non-trivial value", "[ovis][core][Value]") {
//   auto test_module = RegisterValueTestModule();
//   auto shared_double = std::make_shared<double>(8.0);
//   auto other_shared_double = std::make_shared<double>(16.0);
//   REQUIRE(shared_double.use_count() == 1);
//   {
//     ovis::Value value = ovis::Value::Create(shared_double);
//     REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
//     REQUIRE(*value.as<SharedDouble>().get() == 8.0);
//     REQUIRE(shared_double.use_count() == 2);

//     ovis::Value other_value = ovis::Value::Create(other_shared_double);
//     REQUIRE(other_value.type() == ovis::Type::Get<SharedDouble>());
//     REQUIRE(*other_value.as<SharedDouble>().get() == 16.0);
//     REQUIRE(other_shared_double.use_count() == 2);

//     other_value = std::move(value);

//     REQUIRE(other_value.type() == ovis::Type::Get<SharedDouble>());
//     REQUIRE(*other_value.as<SharedDouble>().get() == 8.0);

//     REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
//     REQUIRE(value.as<SharedDouble>() == nullptr);

//     REQUIRE(shared_double.use_count() == 2);
//     REQUIRE(other_shared_double.use_count() == 1);
//   }
//   REQUIRE(shared_double.use_count() == 1);
//   REQUIRE(other_shared_double.use_count() == 1);
// }

TEST_CASE("Construct trivial type", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto number_type = test_module->GetType("Number");
  auto number_value = ovis::Value::Construct(number_type);
  REQUIRE(number_value);
  REQUIRE(number_value->type() == ovis::Type::Get<double>());
  REQUIRE(number_value->as<double>() == 0.0);
}

TEST_CASE("Construct type", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_number_type = test_module->GetType("SharedNumber");
  auto shared_number_value = ovis::Value::Construct(shared_number_type);
  REQUIRE(shared_number_value);
  REQUIRE(shared_number_value->type() == ovis::Type::Get<std::shared_ptr<double>>());
  REQUIRE(shared_number_value->as<std::shared_ptr<double>>() == nullptr);

  auto shared_number = std::make_shared<double>(8.0);
  REQUIRE(shared_number.use_count() == 1);
  shared_number_value->as<SharedDouble>() = shared_number;
  REQUIRE(shared_number.use_count() == 2);
  REQUIRE(shared_number_value->as<SharedDouble>() != nullptr);
  shared_number_value->Reset();
  REQUIRE(shared_number.use_count() == 1);
}

TEST_CASE("Store reference", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto number_type = test_module->GetType("Number");
  double number = 10.0;
  auto value = ovis::Value::Create(&number);
  REQUIRE(value.type() == number_type);
  REQUIRE(value.as<double>() == 10.0);
  value.as<double>() = 12.0;
  REQUIRE(value.as<double>() == 12.0);
  REQUIRE(number == 12.0);
}

