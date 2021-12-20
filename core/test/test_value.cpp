#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/core/value.hpp>

TEST_CASE("Construct trivial value", "[ovis][core][Value]") {
  auto test_module = RegisterTestModule();
  ovis::Value value(8.0);
  REQUIRE(value.as<double>() == 8.0);
}

TEST_CASE("Construct value", "[ovis][core][Value]") {
  auto shared_double = std::make_shared<double>(8.0);
  REQUIRE(shared_double.use_count() == 1);
  {
    ovis::Value value(shared_double);
    REQUIRE(*value.as<std::shared_ptr<double>>().get() == 8.0);
    REQUIRE(shared_double.use_count() == 2);
  }
  REQUIRE(shared_double.use_count() == 1);
}
