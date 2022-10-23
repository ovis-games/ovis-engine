#include "ovis/application/tick_receiver.hpp"
#if OVIS_EMSCRIPTEN
#include "ovis/application/canvas_viewport.hpp"
#include <emscripten.h>
#endif

#include "ovis/utils/profiling.hpp"
#include "ovis/application/sdl_window.hpp"

namespace ovis {

namespace {

bool Update() {
  using namespace std::chrono;
  static auto time_point_of_last_update = high_resolution_clock::now();

  auto now = high_resolution_clock::now();
  auto delta_time = duration_cast<microseconds>(now - time_point_of_last_update);
  time_point_of_last_update += delta_time;

  for (auto tick_receiver : TickReceiver::all()) {
    tick_receiver->Update(delta_time);
  }

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

}  // namespace

void Init() {
  InitializeMainVM();
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
}

}  // namespace ovis
