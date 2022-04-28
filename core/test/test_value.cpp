#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/core/value.hpp>

using SharedDouble = std::shared_ptr<double>;

std::shared_ptr<ovis::Module> RegisterValueTestModule() {
  auto module = RegisterTestModule();
  // module->RegisterType(ovis::TypeDescription::CreateForNativeType<double>("Number"));
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
  auto number_type = ovis::Type::Get<double>();
  ovis::Value number_value(number_type);
  REQUIRE(number_value.type() == ovis::Type::Get<double>());
  REQUIRE(number_value.as<double>() == 0.0);
}

TEST_CASE("Construct type", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_number_type = test_module->GetType("SharedNumber");
  ovis::Value shared_number_value(shared_number_type);
  REQUIRE(shared_number_value.type() == ovis::Type::Get<std::shared_ptr<double>>());
  REQUIRE(shared_number_value.as<std::shared_ptr<double>>() == nullptr);

  auto shared_number = std::make_shared<double>(8.0);
  REQUIRE(shared_number.use_count() == 1);
  shared_number_value.as<SharedDouble>() = shared_number;
  REQUIRE(shared_number.use_count() == 2);
  REQUIRE(shared_number_value.as<SharedDouble>() != nullptr);
  shared_number_value.Reset();
  REQUIRE(shared_number.use_count() == 1);
}

struct SomeReferenceType : public ovis::SafelyReferenceable {
  double number;
};

TEST_CASE("Store reference", "[ovis][core][Value]") {
  auto test_module = RegisterTestModule();

  auto some_reference_type = test_module->RegisterType(ovis::TypeDescription::CreateForNativeType<SomeReferenceType>("SomeReferenceType"));
  REQUIRE(some_reference_type->is_reference_type());

  ovis::Value value(some_reference_type);
  REQUIRE(value.type() == some_reference_type);
  REQUIRE(value.as<SomeReferenceType>().number == 0);
  value.as<SomeReferenceType>().number = 10;
  REQUIRE(value.as<SomeReferenceType>().number == 10);

  // auto value_copy = value;
  // REQUIRE(value.as<SomeReferenceType>().number == 10);

  auto reference_to_value = ovis::Value::Create(&value.as<SomeReferenceType>());
  REQUIRE(reference_to_value.is_reference());
  REQUIRE(reference_to_value.as<SomeReferenceType>().number == 10);

  auto another_reference_to_value = value.CreateReference();
  REQUIRE(another_reference_to_value.is_reference());
  REQUIRE(another_reference_to_value.as<SomeReferenceType>().number == 10);

  value.as<SomeReferenceType>().number = 12;
  // REQUIRE(value_copy.as<SomeReferenceType>().number == 10);
  REQUIRE(reference_to_value.as<SomeReferenceType>().number == 12);
  REQUIRE(another_reference_to_value.as<SomeReferenceType>().number == 12);
}
