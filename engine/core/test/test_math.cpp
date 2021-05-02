#include <catch2/catch.hpp>

#include <ovis/core/math.hpp>

TEST_CASE("GetLineStripInsertPosition", "[ovis][core][math]") {
  const std::vector<ovis::Vector2> strip = {
      {-2.5f, 1.0f}, {-2.0f, 1.0f}, {-1.5f, 1.0f}, {-1.0f, 1.0f}, {0.0f, 1.0f},
  };
  const ovis::Vector2 new_point = {-0.47f, 1.0f};
  REQUIRE(ovis::GetLineStripInsertPosition(strip, new_point) == 4);
}
