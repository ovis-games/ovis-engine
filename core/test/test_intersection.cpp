#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "ovis/core/intersection.hpp"

TEST_CASE("ComputeClosestPointOnLineSegment", "[ovis][core][intersection]") {
  ovis::LineSegment2D line_segment = {{-1.0f, 1.0f}, {0.0f, 1.0f}};
  const ovis::Vector2 point = {-0.47f, 1.0f};
  REQUIRE(ovis::ComputeClosestPointOnLineSegment(line_segment, point).x == Catch::Approx(point.x));
  REQUIRE(ovis::ComputeClosestPointOnLineSegment(line_segment, point).y == Catch::Approx(point.y));
}
