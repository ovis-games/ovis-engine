#include "ovis/sdl/sdl_window.hpp"

#include <algorithm>
#include <cassert>

#include "SDL.h"
#include "SDL_stdinc.h"
#include "SDL_video.h"

#include "ovis/sdl/sdl_event_processor.hpp"
#include "ovis/utils/log.hpp"
#include "ovis/utils/profiling.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/core/simple_job.hpp"
#include "ovis/input/key_events.hpp"
#include "ovis/input/mouse_events.hpp"
#include "ovis/input/text_input_event.hpp"

namespace ovis {

namespace {

SDL_GLContext CreateOpenGLContext(SDL_Window* window) {
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
  LogI("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

  return context;
}

}  // namespace

SDLWindow::SDLWindow(const SDLWindowDescription& desc)
    : sdl_window_(SDL_CreateWindow(desc.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desc.width,
                                   desc.height, SDL_WINDOW_OPENGL)),
      opengl_context_(CreateOpenGLContext(sdl_window_)),
      graphics_context_(GetDrawableSize()),
      is_open_(true) {
  assert(sdl_window_ != nullptr);
  if (desc.scene == nullptr) {
    LogE("Window is not assigned to display a scene");
  } 

  if (!application_scheduler.HasJob("SDLEventProcessor")) {
    application_scheduler.AddJob<SDLEventProcessor>();
  }
  id_ = SDL_GetWindowID(sdl_window());
}

SDLWindow::~SDLWindow() {
  Close();
}

Vector2 SDLWindow::GetDrawableSize() const {
  int width;
  int height;
  SDL_GL_GetDrawableSize(sdl_window_, &width, &height);
  return {static_cast<float>(width), static_cast<float>(height)};
}

void SDLWindow::Close() {
  if (is_open()) {
    SDL_GL_DeleteContext(opengl_context_);
    SDL_DestroyWindow(sdl_window_);
    is_open_ = false;
  }
}

void SDLWindow::Resize(int width, int height) {
  SDL_SetWindowSize(sdl_window_, width, height);
}

void SDLWindow::ProcessEvent(const SDL_Event& event) {
  if (event.type == SDL_WINDOWEVENT) {
    switch (event.window.event) {
      case SDL_WINDOWEVENT_CLOSE:
        Close();
        break;
    }
  }

  const auto sdl_button_to_mouse_button = [](uint8_t sdl_button) {
    switch (sdl_button) {
      case SDL_BUTTON_LEFT:
        return MouseButton::Left();
      case SDL_BUTTON_MIDDLE:
        return MouseButton::Middle();
      case SDL_BUTTON_RIGHT:
        return MouseButton::Right();
      default:
        SDL_assert(false);
        return MouseButton::Left();
    }
  };

// #if !OVIS_EMSCRIPTEN
//   switch (event.type) {
//     case SDL_MOUSEWHEEL: {
//       MouseWheelEvent mouse_wheel_event({static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y)});
//       ProcessEvent(&mouse_wheel_event);
//       return !mouse_wheel_event.is_propagating();
//     }

//     case SDL_MOUSEMOTION: {
//       MouseMoveEvent mouse_move_event(this, {static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)},
//                                       {static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel)});
//       ProcessEvent(&mouse_move_event);
//       return !mouse_move_event.is_propagating();
//     }

//     case SDL_MOUSEBUTTONDOWN: {
//       const auto button = sdl_button_to_mouse_button(event.button.button);
//       MouseButtonPressEvent mouse_button_event(
//           this, {static_cast<float>(event.button.x), static_cast<float>(event.button.y)}, button);
//       SetMouseButtonState(button, true);
//       ProcessEvent(&mouse_button_event);
//       return !mouse_button_event.is_propagating();
//     }

//     case SDL_MOUSEBUTTONUP: {
//       const auto button = sdl_button_to_mouse_button(event.button.button);
//       MouseButtonReleaseEvent mouse_button_event(
//           this, {static_cast<float>(event.button.x), static_cast<float>(event.button.y)}, button);
//       SetMouseButtonState(button, false);
//       ProcessEvent(&mouse_button_event);
//       return !mouse_button_event.is_propagating();
//     }

//     case SDL_TEXTINPUT: {
//       TextInputEvent text_input_event(event.text.text);
//       ProcessEvent(&text_input_event);
//       return !text_input_event.is_propagating();
//     }

//     case SDL_KEYDOWN: {
//       KeyPressEvent key_press_event(Key{static_cast<uint16_t>(event.key.keysym.scancode)});
//       SetKeyState(key_press_event.key(), true);
//       ProcessEvent(&key_press_event);
//       return !key_press_event.is_propagating();
//     }

//     case SDL_KEYUP: {
//       KeyReleaseEvent key_release_event(Key{static_cast<uint16_t>(event.key.keysym.scancode)});
//       SetKeyState(key_release_event.key(), false);
//       ProcessEvent(&key_release_event);
//       return !key_release_event.is_propagating();
//     }
//   }
// #endif
}

SDLWindow* SDLWindow::GetWindowById(Uint32 id) {
  for (auto window : all()) {
    if (window->id() == id) {
      return window;
    }
  }
  return nullptr;
}

}  // namespace ovis
