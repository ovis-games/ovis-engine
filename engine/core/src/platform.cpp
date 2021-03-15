#include <string>

#include <emscripten/val.h>

#include <ovis/core/platform.hpp>

namespace ovis {

#if OVIS_EMSCRIPTEN
namespace {

Platform GetEmscriptenPlatform() {
  const std::string app_version = emscripten::val::global("navigator")["appVersion"].as<std::string>();
  if (app_version.find("Win") != std::string::npos) {
    return Platform::WINDOWS;
  } else if (app_version.find("Mac") != std::string::npos) {
    return Platform::MACOS;
  } else if (app_version.find("Linux") != std::string::npos) {
    return Platform::LINUX;
  } else {
    return Platform::UNKNOWN;
  }
}

}  // namespace
#endif

Platform GetPlatform() {
#if OVIS_EMSCRIPTEN
  static const Platform platform = GetEmscriptenPlatform();
  return platform;
#elif defined(_WIN32)
  return Platform::WINDOWS;
#elif defined(__linux__)
  return Platform::LINUX;
#elif defined(__APPLE__)
  return Platform::MACOS;
#else
  return Platform::UNKNOWN;
#endif
}

}  // namespace ovis