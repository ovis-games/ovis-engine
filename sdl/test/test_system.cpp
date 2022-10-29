#include "catch2/catch_test_macros.hpp"

#include "ovis/sdl/sdl_init_subsystem.hpp"
#include "ovis/sdl/system.hpp"
#include "ovis/utils/log.hpp"

using namespace ovis;

TEST_CASE("Quering displays", "[ovis][sdl][System]") {
  const auto displays = GetDisplays();

  LogI("Display count: {}", displays.size());
  for (const auto& display : displays) {
    LogI("Display: {}", display);
  }
}
