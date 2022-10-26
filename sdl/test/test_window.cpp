#include "catch2/catch_test_macros.hpp"
#include "ovis/sdl/sdl_window.hpp"
#include "ovis/core/application.hpp"
#include "ovis/core/scene.hpp"

using namespace ovis;

TEST_CASE("Create sdl window", "[ovis][sdl][SDLWindow]") {
  Scene scene;

  SDLWindow window({ .title = "Test window", .scene = &scene });
  SDLWindow window2({ .title = "Test window 2", .scene = &scene });

  // RunApplicationLoop();
}
