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
  auto window = SDL_CreateWindow("ClearPass", 100, 100, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  auto gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);

  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  GraphicsContext context(Vector2(width, height));

  Scene scene;
  scene.frame_scheduler().AddJob<ClearPass>(&context, Color::Aqua());
  scene.Prepare();
  scene.Play();
  scene.Update(0.0);

  std::vector<uint32_t> framebuffer(100 * 100);
  glReadPixels(0, 0, 100, 100, GL_RGBA,  GL_UNSIGNED_BYTE, framebuffer.data());

  for (int y = 0; y < 100; ++y) {
    for (int x = 0; x < 100; ++ x) {
      REQUIRE(framebuffer[y * 100 + x] == ConvertToRGBA8(Color::Aqua()));
    }
  }
}
