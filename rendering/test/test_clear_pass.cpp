#include <chrono>
#include <string>
#include <thread>

#include "catch2/catch_test_macros.hpp"

#include "ovis/core/scene.hpp"
#include "ovis/graphics/gl.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/rendering/clear_pass.hpp"
#include "ovis/test/require_result.hpp"
#include "ovis/test/test_window.hpp"

using namespace ovis;

TEST_CASE("Test clear pass", "[ovis][rendering][ClearPass]") {
  test::TestWindow window;
  Scene scene;
  scene.frame_scheduler().AddJob<ClearPass>(&window.graphics_context, Color::Aqua());
  REQUIRE_RESULT(scene.Prepare());
  scene.Play();
  scene.Update(0.0);

  for (const auto pixel : window.GetFramebufferContent()) {
    REQUIRE(pixel == ConvertToRGBA8(Color::Aqua()));
  }
}
