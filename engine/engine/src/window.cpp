#include <algorithm>
#include <cassert>

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

std::vector<Window*> Window::all_windows_;

Window::Window(const WindowDescription& desc)
    : sdl_window_(SDL_CreateWindow(desc.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desc.width,
                                   desc.height, SDL_WINDOW_OPENGL)),
      id_(SDL_GetWindowID(sdl_window_)),
      graphics_context_(sdl_window_),
      scene_() {
  assert(sdl_window_ != nullptr);
  all_windows_.push_back(this);

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

glm::ivec2 Window::GetSize() {
  glm::ivec2 window_size;
  SDL_GetWindowSize(sdl_window_, &window_size.x, &window_size.y);
  return window_size;
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

  switch (event.type) {
    case SDL_MOUSEWHEEL: {
      MouseWheelEvent mouse_wheel_event(event.wheel.x, event.wheel.y);
      scene_.ProcessEvent(&mouse_wheel_event);
      return !mouse_wheel_event.is_propagating();
    }

    case SDL_MOUSEMOTION: {
      MouseMoveEvent mouse_move_event(this, {event.motion.x, event.motion.y}, {event.motion.xrel, event.motion.yrel});
      scene_.ProcessEvent(&mouse_move_event);
      return !mouse_move_event.is_propagating();
    }

    case SDL_MOUSEBUTTONDOWN: {
      MouseButtonPressEvent mouse_button_event(this, {event.button.x, event.button.y},
                                               static_cast<MouseButton>(event.button.button));
      scene_.ProcessEvent(&mouse_button_event);
      return !mouse_button_event.is_propagating();
    }

    case SDL_MOUSEBUTTONUP: {
      MouseButtonReleaseEvent mouse_button_event(this, {event.button.x, event.button.y},
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