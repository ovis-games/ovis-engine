#include "catch2/catch_test_macros.hpp"

#include "ovis/utils/utf8.hpp"

TEST_CASE("replace_all", "[ovis][utils][string]") {
  using namespace ovis;

  REQUIRE(replace_all("test/test", "/", "/abc/") == "test/abc/test");
}
