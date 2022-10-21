#include "SDL_video.h"
#include "catch2/catch.hpp"
#include <chrono>
#include <string>
#include <thread>

#include "ovis/core/scene.hpp"
#include "ovis/graphics/gl.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/rendering/clear_pass.hpp"
#include "ovis/test/utils.hpp"

using namespace ovis;

TEST_CASE("Test clear pass", "[ovis][rendering][ClearPass]") {
  test::TestWindow window;
  Scene scene;
  scene.frame_scheduler().AddJob<ClearPass>(&window.graphics_context, Color::Aqua());
  scene.Prepare();
  scene.Play();
  scene.Update(0.0);

  for (const auto pixel : window.GetFramebufferContent()) {
    REQUIRE(pixel == ConvertToRGBA8(Color::Aqua()));
  }
}
