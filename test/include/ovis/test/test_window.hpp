#if __has_include("ovis/graphics/graphics_context.hpp")
#include "SDL_video.h"
#include "ovis/graphics/graphics_context.hpp"

// Define the namespace to make the statement 'using namespace ovis' not an error in snippets.
namespace ovis {
namespace test {

namespace detail {

inline SDL_GLContext CreateOpenGLContext(SDL_Window* window) {
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#if !defined(__IPHONEOS__) && !defined(__EMSCRIPTEN__)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

  auto context = SDL_GL_CreateContext(window);
  SDL_assert(context != nullptr);
  SDL_GL_MakeCurrent(window, context);
  LogI("OpenGL version: {}", glGetString(GL_VERSION));

  return context;
}

}  // namespace

struct TestWindow {
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
  std::unique_ptr<void, decltype(&SDL_GL_DeleteContext)> gl_context;
  GraphicsContext graphics_context;
  int client_width;
  int client_height;

  TestWindow(int width = 1280, int height = 720, bool show = false)
      : window(SDL_CreateWindow(Catch::getResultCapture().getCurrentTestName().c_str(), 0, 0, width, height,
                                show ? SDL_WINDOW_OPENGL : SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN),
               SDL_DestroyWindow),
        gl_context(detail::CreateOpenGLContext(window.get()), SDL_GL_DeleteContext),
        graphics_context(Vector2(1280, 720)) {
    SDL_GetWindowSize(window.get(), &client_width, &client_height);
    graphics_context.SetFramebufferSize(client_width, client_height);
  }

  std::vector<uint32_t> GetFramebufferContent() {
    std::vector<uint32_t> framebuffer(client_width * client_height);
    glReadPixels(0, 0, client_width, client_height, GL_RGBA,  GL_UNSIGNED_BYTE, framebuffer.data());
    return framebuffer;
  }

  void Swap() {
    SDL_GL_SwapWindow(window.get());
  }
};

}  // namespace test
}  // namespace ovis

#endif
