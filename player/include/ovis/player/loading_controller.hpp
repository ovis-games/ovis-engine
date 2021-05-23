#pragma once

#if OVIS_EMSCRIPTEN

#include <cstddef>
#include <string_view>
#include <vector>

#include <microtar.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {
namespace player {

class LoadingController : public SceneController {
  enum class State { DOWNLOADING_PACKAGE, EXTRACTING, ERROR };

 public:
  LoadingController(std::string_view backend_prefix, std::string_view game_id, std::string_view package_type);

  void Update(std::chrono::microseconds delta_time) override;

 private:
  float progress_ = 0.0f;
  std::vector<std::byte> package_;
  std::vector<std::byte> file_buffer_;
  State state_ = State::DOWNLOADING_PACKAGE;
  mtar_t tar_;
};

}  // namespace player
}  // namespace ovis
#endif

