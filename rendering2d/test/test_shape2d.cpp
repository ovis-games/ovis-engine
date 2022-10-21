#include "catch2/catch.hpp"

#include "ovis/rendering2d/shape2d.hpp"

using namespace ovis;

TEST_CASE("Deserialize scene object with Shape2D", "[ovis][rendering2d][Shape2d]") {
  Shape2D shape = R"(
  {
    "color": [0.5, 1, 0.5, 1.0]
  }
  )"_json;
  REQUIRE(shape.color() == Color(0.5, 1.0, 0.5, 1.0));
}
