#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/core/value.hpp>

using SharedDouble = std::shared_ptr<double>;

std::shared_ptr<ovis::Module> RegisterValueTestModule() {
  auto module = RegisterTestModule();
  module->RegisterType<double>("Number");
  module->RegisterType<SharedDouble>("SharedNumber");
  return module;
}

TEST_CASE("Construct trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  ovis::Value value(8.0);
  REQUIRE(value.type() == ovis::Type::Get<double>());
  REQUIRE(value.as<double>() == 8.0);
}

TEST_CASE("Construct non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value(shared_double);
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
    ovis::Value value(shared_double);
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

TEST_CASE("Move non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value(shared_double);
    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);
    {
      ovis::Value value_copy(std::move(value));
      REQUIRE(value.as<SharedDouble>() == nullptr);
      REQUIRE(shared_double.use_count() == 2);

      REQUIRE(value_copy.type() == ovis::Type::Get<SharedDouble>());
      REQUIRE(*value_copy.as<SharedDouble>().get() == 8.0);
    }
    REQUIRE(shared_double.use_count() == 1);
  }
  REQUIRE(shared_double.use_count() == 1);
}

TEST_CASE("Copy assign non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  auto other_shared_double = std::make_shared<double>(16.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value(shared_double);
    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);

    ovis::Value other_value(other_shared_double);
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

TEST_CASE("Move assign non-trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterValueTestModule();
  auto shared_double = std::make_shared<double>(8.0);
  auto other_shared_double = std::make_shared<double>(16.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value(shared_double);
    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*value.as<SharedDouble>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);

    ovis::Value other_value(other_shared_double);
    REQUIRE(other_value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*other_value.as<SharedDouble>().get() == 16.0);
    REQUIRE(other_shared_double.use_count() == 2);

    other_value = std::move(value);

    REQUIRE(other_value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(*other_value.as<SharedDouble>().get() == 8.0);

    REQUIRE(value.type() == ovis::Type::Get<SharedDouble>());
    REQUIRE(value.as<SharedDouble>() == nullptr);

    REQUIRE(shared_double.use_count() == 2);
    REQUIRE(other_shared_double.use_count() == 1);
  }
  REQUIRE(shared_double.use_count() == 1);
  REQUIRE(other_shared_double.use_count() == 1);
}

