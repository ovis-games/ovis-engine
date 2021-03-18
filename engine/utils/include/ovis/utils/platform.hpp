#pragma once

namespace ovis {

enum class Platform {
  UNKNOWN,
  LINUX,
  WINDOWS,
  MACOS,
};

Platform GetPlatform();

}  // namespace ovis
