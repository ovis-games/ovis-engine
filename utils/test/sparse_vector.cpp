#include <catch2/catch.hpp>

#include <ovis/utils/sparse_vector.hpp>

TEST_CASE("sparse_vector", "[ovis][utils]") {
  using namespace ovis;

  sparse_vector<int> vector;
  REQUIRE(vector.size() == 0);
  REQUIRE(vector.capacity() == 0);
  REQUIRE(vector.begin() == vector.end());
  REQUIRE(vector.empty());

  REQUIRE(vector.insert(1337) == vector.begin());
  REQUIRE(vector.size() == 1);
  REQUIRE(vector.capacity() >= 1);
  REQUIRE(vector.contains(0));
  REQUIRE(vector.at(0) == 1337);
  REQUIRE(vector[0] == 1337);
  REQUIRE(*vector.begin() == 1337);
  REQUIRE(vector.begin() != vector.end());
  REQUIRE(++vector.begin() == vector.end());
  REQUIRE(!vector.empty());

  REQUIRE(vector.insert(42) == ++vector.begin());
  REQUIRE(vector.size() == 2);
  REQUIRE(vector.capacity() >= 2);
  REQUIRE(vector.contains(0));
  REQUIRE(vector.contains(1));
  REQUIRE(vector.at(0) == 1337);
  REQUIRE(vector[0] == 1337);
  REQUIRE(vector.at(1) == 42);
  REQUIRE(vector[1] == 42);
  REQUIRE(*vector.begin() == 1337);
  REQUIRE(vector.begin() != vector.end());

  for (auto x : vector) {
    const bool is_valid = x == 1337 || x == 42;
    REQUIRE(is_valid);
  }

  // Erase 1337
  vector.erase(vector.begin());
  REQUIRE(vector.size() == 1);
  REQUIRE(vector.capacity() >= 2);
  REQUIRE(!vector.contains(0));
  REQUIRE(vector.contains(1));
  REQUIRE(vector.at(1) == 42);
  REQUIRE(vector[1] == 42);
  REQUIRE(*vector.begin() == 42);
  REQUIRE(vector.begin() != vector.end());

  // Erase 42
  vector.erase(vector.begin());
  REQUIRE(vector.size() == 0);
  REQUIRE(vector.capacity() >= 2);
  REQUIRE(!vector.contains(0));
  REQUIRE(!vector.contains(1));
  REQUIRE(vector.begin() == vector.end());
}
