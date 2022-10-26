#include "ovis/core/application.hpp"

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#endif

namespace ovis {

ApplicationSchedular application_scheduler;

namespace {

bool quit = false;

void Update() {
  using namespace std::chrono;
  static auto time_point_of_last_update = high_resolution_clock::now();

  auto now = high_resolution_clock::now();
  const double delta_time = duration<double>(now - time_point_of_last_update).count();
  time_point_of_last_update = now;

  application_scheduler(delta_time);

#if OVIS_ENABLE_BUILT_IN_PROFILING
  ProfilingLog::default_log()->AdvanceFrame();
#endif
}

#ifdef OVIS_EMSCRIPTEN
void EmscriptenUpdate() {
  if (quit) {
    emscripten_cancel_main_loop();
  } else {
    Update();
  }
}
#endif  // OVIS_EMSCRIPTEN

}  // namespace

void RunApplicationLoop() {
#if OVIS_EMSCRIPTEN
  emscripten_set_main_loop(&EmscriptenUpdate, 0, true);
#else
  while (!quit) {
    Update();
  }
#endif
}

}  // namespace ovis
