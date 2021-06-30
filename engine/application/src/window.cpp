#include <algorithm>
#include <cassert>

#if OVIS_EMSCRIPTEN
#include <emscripten/html5.h>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/utils/profiling.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/input/emscripten_callbacks.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/input/mouse_events.hpp>
#include <ovis/input/text_input_event.hpp>
#include <ovis/application/window.hpp>

namespace ovis {

std::vector<Window*> Window::all_windows_;

Window::Window(const WindowDescription& desc)
    : sdl_window_(SDL_CreateWindow(desc.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desc.width,
                                   desc.height, SDL_WINDOW_OPENGL)),
      id_(SDL_GetWindowID(sdl_window_)),
      graphics_context_(sdl_window_),
      scene_() {
  assert(sdl_window_ != nullptr);
  all_windows_.push_back(this);

#if OVIS_EMSCRIPTEN
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
  emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);

  emscripten_set_mousemove_callback("canvas", nullptr, 0, nullptr);
  emscripten_set_mousedown_callback("canvas", nullptr, 0, nullptr);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, nullptr);
  emscripten_set_mouseenter_callback("canvas", nullptr, 0, nullptr);
  emscripten_set_mouseleave_callback("canvas", nullptr, 0, nullptr);
  emscripten_set_wheel_callback("canvas", nullptr, 0, nullptr);

  emscripten_set_keydown_callback("canvas", static_cast<SceneViewport*>(this), 0, &HandleKeyDownEvent);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, static_cast<SceneViewport*>(this), 0,
                                &HandleKeyUpEvent);
  emscripten_set_keypress_callback("canvas", static_cast<SceneViewport*>(this), 0, &HandleKeyPressEvent);

  emscripten_set_mousemove_callback("canvas", static_cast<SceneViewport*>(this), 0, &HandleMouseMoveEvent);
  emscripten_set_mousedown_callback("canvas", static_cast<SceneViewport*>(this), 0, &HandleMouseDownEvent);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, static_cast<SceneViewport*>(this), 0,
                                  &HandleMouseUpEvent);
  // emscripten_set_mouseenter_callback("canvas", nullptr, 0, HandleMouseEvent);
  // emscripten_set_mouseleave_callback("canvas", nullptr, 0, HandleMouseEvent);
  emscripten_set_wheel_callback("canvas", static_cast<SceneViewport*>(this), 0, &HandleWheelEvent);
  emscripten_set_blur_callback("canvas", static_cast<SceneViewport*>(this), 0, &HandleBlurEvent);
#endif

  SetGraphicsContext(&graphics_context_);
  SetScene(&scene_);
  scene_.SetMainViewport(this);
}

Window::~Window() {
  auto it = std::find(all_windows_.begin(), all_windows_.end(), this);
  assert(it != all_windows_.end());
  std::swap(*it, all_windows_.back());
  all_windows_.pop_back();

  ClearResources();
  SDL_DestroyWindow(sdl_window_);
}

RenderTargetConfiguration* Window::GetDefaultRenderTargetConfiguration() {
  return graphics_context_.default_render_target_configuration();
}

Vector2 Window::GetDimensions() const {
  int width;
  int height;
  SDL_GetWindowSize(sdl_window_, &width, &height);
  return {static_cast<float>(width), static_cast<float>(height)};
}

void Window::Resize(int width, int height) {
  SDL_SetWindowSize(sdl_window_, width, height);
}

bool Window::SendEvent(const SDL_Event& event) {
  if (event.type == SDL_WINDOWEVENT) {
    switch (event.window.event) {
      case SDL_WINDOWEVENT_CLOSE:
        is_open_ = false;
        break;
    }
  }

  const auto sdl_button_to_mouse_button = [](uint8_t sdl_button) {
    switch (sdl_button) {
      case SDL_BUTTON_LEFT: return MouseButton::Left();
      case SDL_BUTTON_MIDDLE: return MouseButton::Middle();
      case SDL_BUTTON_RIGHT: return MouseButton::Right();
      default: SDL_assert(false); return MouseButton::Left();
    }
  };

#if !OVIS_EMSCRIPTEN
  switch (event.type) {
    case SDL_MOUSEWHEEL: {
      MouseWheelEvent mouse_wheel_event({static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y)});
      scene_.ProcessEvent(&mouse_wheel_event);
      return !mouse_wheel_event.is_propagating();
    }

    case SDL_MOUSEMOTION: {
      MouseMoveEvent mouse_move_event(this, {static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)},
                                      {static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel)});
      scene_.ProcessEvent(&mouse_move_event);
      return !mouse_move_event.is_propagating();
    }

    case SDL_MOUSEBUTTONDOWN: {
      const auto button = sdl_button_to_mouse_button(event.button.button);
      MouseButtonPressEvent mouse_button_event(this,
                                               {static_cast<float>(event.button.x), static_cast<float>(event.button.y)},
                                               button);
      SetMouseButtonState(button, true);
      scene_.ProcessEvent(&mouse_button_event);
      return !mouse_button_event.is_propagating();
    }

    case SDL_MOUSEBUTTONUP: {
      const auto button = sdl_button_to_mouse_button(event.button.button);
      MouseButtonReleaseEvent mouse_button_event(
          this, {static_cast<float>(event.button.x), static_cast<float>(event.button.y)},
          button);
      SetMouseButtonState(button, false);
      scene_.ProcessEvent(&mouse_button_event);
      return !mouse_button_event.is_propagating();
    }

    case SDL_TEXTINPUT: {
      TextInputEvent text_input_event(event.text.text);
      scene_.ProcessEvent(&text_input_event);
      return !text_input_event.is_propagating();
    }

    case SDL_KEYDOWN: {
      KeyPressEvent key_press_event(Key{static_cast<uint16_t>(event.key.keysym.scancode)});
      SetKeyState(key_press_event.key(), true);
      scene_.ProcessEvent(&key_press_event);
      return !key_press_event.is_propagating();
    }

    case SDL_KEYUP: {
      KeyReleaseEvent key_release_event(Key{static_cast<uint16_t>(event.key.keysym.scancode)});
      SetKeyState(key_release_event.key(), false);
      scene_.ProcessEvent(&key_release_event);
      return !key_release_event.is_propagating();
    }
  }
#endif

  return false;
}

void Window::Update(std::chrono::microseconds delta_time) {
  if (!scene_.is_playing()) {
    scene_.Play();
  }
  scene_.BeforeUpdate();
  scene_.Update(delta_time);
  scene_.AfterUpdate();
}

void Window::Render() {
  RenderingViewport::Render();
  SDL_GL_SwapWindow(sdl_window_);
}

}  // namespace ovis
