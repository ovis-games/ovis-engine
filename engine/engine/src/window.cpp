#include <algorithm>
#include <cassert>

#if OVIS_EMSCRIPTEN
#include <emscripten/html5.h>
#endif

#include <ovis/core/log.hpp>
#include <ovis/core/profiling.hpp>
#include <ovis/graphics/cubemap.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/window.hpp>

namespace ovis {

#if OVIS_EMSCRIPTEN

static EM_BOOL HandleWheelEvent(int event_type, const EmscriptenWheelEvent* wheel_event, void* user_data) {
  Window* window = static_cast<Window*>(user_data);
  MouseWheelEvent mouse_wheel_event(
      {static_cast<float>(-wheel_event->deltaX), static_cast<float>(-wheel_event->deltaY)});
  window->scene()->ProcessEvent(&mouse_wheel_event);
  return !mouse_wheel_event.is_propagating();
}

EM_BOOL HandleKeyDownEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* user_data) {
  const Key key = Key::FromName(keyboard_event->code);
  input()->SetKeyState(static_cast<SDL_Scancode>(key.code), true);

  Window* window = static_cast<Window*>(user_data);

  bool key_press_event_is_propagating = false;
  if (keyboard_event->repeat == false) {
    KeyPressEvent key_press_event(key);
    window->scene()->ProcessEvent(&key_press_event);
    key_press_event_is_propagating = !key_press_event.is_propagating();
  }

  // Always prevent default action for the tab key
  if (key == Key::TAB) {
    return true;
  }

  // Never prevent default action for CTRL+V or Insert key (Paste)
  if ((key == Key::KEY_V && keyboard_event->ctrlKey) || key == Key::INSERT) {
    return false;
  }

  // Never prevent default action for CTRL+C (Copy)
  if (key == Key::KEY_C && keyboard_event->ctrlKey) {
    return false;
  }

  return !key_press_event_is_propagating;
}

// TODO: the keypress event is actually deprecated and should be replaced by the keydown event. But there is no easy way
// to check whether
EM_BOOL HandleKeyPressEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* user_data) {
  Window* window = static_cast<Window*>(user_data);

  // For some reason keyboard_event->key contains "Enter" when pressing the enter button
  if (std::strcmp("Enter", keyboard_event->key) != 0) {
    TextInputEvent text_input_event(keyboard_event->key);
    window->scene()->ProcessEvent(&text_input_event);
    return !text_input_event.is_propagating();
  } else {
    return false;
  }
}

EM_BOOL HandleKeyUpEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* user_data) {
  const Key key = Key::FromName(keyboard_event->code);
  input()->SetKeyState(static_cast<SDL_Scancode>(key.code), false);

  Window* window = static_cast<Window*>(user_data);
  KeyReleaseEvent key_release_event(key);
  window->scene()->ProcessEvent(&key_release_event);
  return !key_release_event.is_propagating();
}

EM_BOOL HandleMouseMoveEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  Window* window = static_cast<Window*>(user_data);
  MouseMoveEvent mouse_move_event(
      window, {static_cast<float>(mouse_event->targetX), static_cast<float>(mouse_event->targetY)},
      {static_cast<float>(mouse_event->movementX), static_cast<float>(mouse_event->movementY)});
  window->scene()->ProcessEvent(&mouse_move_event);
  return !mouse_move_event.is_propagating();
}

EM_BOOL HandleMouseDownEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  input()->SetMouseButtonState(static_cast<MouseButton>(mouse_event->button), true);

  Window* window = static_cast<Window*>(user_data);
  MouseButtonPressEvent mouse_button_event(
      window, {static_cast<float>(mouse_event->targetX), static_cast<float>(mouse_event->targetY)},
      static_cast<MouseButton>(mouse_event->button));
  window->scene()->ProcessEvent(&mouse_button_event);
  return false;
}

EM_BOOL HandleMouseUpEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  input()->SetMouseButtonState(static_cast<MouseButton>(mouse_event->button), false);

  Window* window = static_cast<Window*>(user_data);
  MouseButtonReleaseEvent mouse_button_event(
      window, {static_cast<float>(mouse_event->targetX), static_cast<float>(mouse_event->targetY)},
      static_cast<MouseButton>(mouse_event->button));
  window->scene()->ProcessEvent(&mouse_button_event);
  return false;
}

EM_BOOL HandleMouseEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  return false;
}

#endif

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

  emscripten_set_keydown_callback("canvas", this, 0, &HandleKeyDownEvent);
  emscripten_set_keyup_callback("canvas", this, 0, &HandleKeyUpEvent);
  emscripten_set_keypress_callback("canvas", this, 0, &HandleKeyPressEvent);

  emscripten_set_mousemove_callback("canvas", this, 0, &HandleMouseMoveEvent);
  emscripten_set_mousedown_callback("canvas", this, 0, &HandleMouseDownEvent);
  emscripten_set_mouseup_callback("canvas", this, 0, &HandleMouseUpEvent);
  // emscripten_set_mouseenter_callback("canvas", nullptr, 0, HandleMouseEvent);
  // emscripten_set_mouseleave_callback("canvas", nullptr, 0, HandleMouseEvent);
  emscripten_set_wheel_callback("canvas", this, 0, &HandleWheelEvent);
#endif

  // resource_manager_.RegisterFileLoader(
  //     ".texture2d", std::bind(&LoadTexture2D, &graphics_context_, std::placeholders::_1, std::placeholders::_2,
  //                             std::placeholders::_3, std::placeholders::_4));

  // resource_manager_.RegisterFileLoader(
  //     ".cubemap", std::bind(&LoadCubemap, &graphics_context_, std::placeholders::_1, std::placeholders::_2,
  //                           std::placeholders::_3, std::placeholders::_4));

  // resource_manager_.RegisterFileLoader(
  //     ".shader", std::bind(&LoadShaderProgram, &graphics_context_, std::placeholders::_1,
  //     std::placeholders::_2,
  //                          std::placeholders::_3, std::placeholders::_4));

  for (const auto& search_path : desc.resource_search_paths) {
    resource_manager_.AddSearchPath(search_path);
  }

  SetResourceManager(&resource_manager_);
  SetGraphicsContext(&graphics_context_);

  for (const auto& controller : desc.scene_controllers) {
    scene_.AddController(controller);
  }

  for (const auto& render_pass : desc.render_passes) {
    AddRenderPass(render_pass);
  }

  SetScene(&scene_);
}

Window::~Window() {
  auto it = std::find(all_windows_.begin(), all_windows_.end(), this);
  assert(it != all_windows_.end());
  std::swap(*it, all_windows_.back());
  all_windows_.pop_back();

  SDL_DestroyWindow(sdl_window_);
}

RenderTargetConfiguration* Window::GetDefaultRenderTargetConfiguration() {
  return graphics_context_.default_render_target_configuration();
}

void Window::GetDimensions(size_t* width, size_t* height) {
  int int_width;
  int int_height;
  SDL_GetWindowSize(sdl_window_, &int_width, &int_height);
  *width = static_cast<size_t>(int_width);
  *height = static_cast<size_t>(int_height);
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

#if !OVIS_EMSCRIPTEN
  switch (event.type) {
    case SDL_MOUSEWHEEL: {
      MouseWheelEvent mouse_wheel_event(event.wheel.x, event.wheel.y);
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
      MouseButtonPressEvent mouse_button_event(this,
                                               {static_cast<float>(event.button.x), static_cast<float>(event.button.y)},
                                               static_cast<MouseButton>(event.button.button));
      scene_.ProcessEvent(&mouse_button_event);
      return !mouse_button_event.is_propagating();
    }

    case SDL_MOUSEBUTTONUP: {
      MouseButtonReleaseEvent mouse_button_event(
          this, {static_cast<float>(event.button.x), static_cast<float>(event.button.y)},
          static_cast<MouseButton>(event.button.button));
      scene_.ProcessEvent(&mouse_button_event);
      return !mouse_button_event.is_propagating();
    }

    case SDL_TEXTINPUT: {
      TextInputEvent text_input_event(event.text.text);
      scene_.ProcessEvent(&text_input_event);
      return !text_input_event.is_propagating();
    }

    case SDL_KEYDOWN: {
      KeyPressEvent key_press_event({static_cast<uint16_t>(event.key.keysym.scancode)});
      scene_.ProcessEvent(&key_press_event);
      return !key_press_event.is_propagating();
    }

    case SDL_KEYUP: {
      KeyReleaseEvent key_release_event({static_cast<uint16_t>(event.key.keysym.scancode)});
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

void Window::Render(bool render_gui) {
  Viewport::Render(render_gui);
  SDL_GL_SwapWindow(sdl_window_);
}

}  // namespace ovis