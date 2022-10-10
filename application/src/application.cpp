#include <SDL2/SDL.h>
#include "ovis/application/tick_receiver.hpp"
#if OVIS_EMSCRIPTEN
#include "ovis/application/canvas_viewport.hpp"
#include <emscripten.h>
#endif

#include "ovis/utils/profiling.hpp"
#include "ovis/core/lua.hpp"
#include "ovis/application/sdl_window.hpp"

namespace ovis {

namespace {

bool ProcessEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      return false;
    }

    // TODO: implement
    // if (event.type == SDL_KEYDOWN) {
    //   input()->SetKeyState(event.key.keysym.scancode, true);
    // } else if (event.type == SDL_KEYUP) {
    //   input()->SetKeyState(event.key.keysym.scancode, false);
    // } else if (event.type == SDL_MOUSEBUTTONDOWN) {
    //   input()->SetMouseButtonState(static_cast<MouseButton>(event.button.button), true);
    // } else if (event.type == SDL_MOUSEBUTTONUP) {
    //   input()->SetMouseButtonState(static_cast<MouseButton>(event.button.button), false);
    // }

    // TODO: only post events to the according window
    for (auto window : Window::all_windows()) {
      window->SendEvent(event);
    }
  }

  return true;
}

bool Update() {
  using namespace std::chrono;
  static auto time_point_of_last_update = high_resolution_clock::now();

  auto now = high_resolution_clock::now();
  auto delta_time = duration_cast<microseconds>(now - time_point_of_last_update);
  time_point_of_last_update += delta_time;

  if (!ProcessEvents()) {
    return false;
  }

  for (auto tick_receiver : TickReceiver::all()) {
    tick_receiver->Update(delta_time);
  }
  for (auto window : Window::all_windows()) {
    window->Render();
  }

#if OVIS_EMSCRIPTEN
  for (auto canvas_viewport : CanvasViewport::all()) {
    canvas_viewport->Render();
  }
#endif

#if OVIS_ENABLE_BUILT_IN_PROFILING
  ProfilingLog::default_log()->AdvanceFrame();
#endif

  return true;
}

#ifdef OVIS_EMSCRIPTEN
void EmscriptenUpdate() {
  try {
    if (!Update()) {
      LogI("Quitting application!");
      emscripten_cancel_main_loop();
    }
  } catch (const std::exception& error) {
    LogE("An unhandled exception occured: {}", error.what());
  } catch (...) {
    LogE("An unhandled exception occured");
  }
}
#endif  // OVIS_EMSCRIPTEN

void LuaPanic(sol::optional<std::string_view> message) {
  LogE("Lua panic!");
  if (message) {
    LogE("{}", *message);
  }
  LuaErrorEvent event(message.value_or(""));
  for (auto window : Window::all_windows()) {
    window->scene()->ProcessEvent(&event);
  }
}

void LuaError(sol::optional<std::string_view> message) {
  LogE("Lua error");
  if (message) {
    LogE("{}", *message);
  }
  LuaErrorEvent event(message.value_or(""));
  for (auto window : Window::all_windows()) {
    window->scene()->ProcessEvent(&event);
  }
}

}  // namespace

void Init() {
  InitializeMainVM();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    LogE("Failed to initialize SDL: {}", SDL_GetError());
  }
  lua.set_panic(sol::c_call<decltype(&LuaPanic), &LuaPanic>);

  lua["OvisErrorHandler"] = LuaError;
  sol::protected_function::set_default_handler(lua["OvisErrorHandler"]);
}

void Run() {
#if OVIS_EMSCRIPTEN
  emscripten_set_main_loop(&EmscriptenUpdate, 0, true);
#else
  while (Update())
    ;
#endif
}

void Quit() {
  SDL_Quit();
}

}  // namespace ovis
