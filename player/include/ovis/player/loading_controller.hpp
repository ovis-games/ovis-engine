#pragma once

#include <cstddef>
#include <vector>

#include <microtar.h>

#include <ovis/engine/scene_controller.hpp>

namespace ovis {
namespace player {

class LoadingController : public ovis::SceneController {
  enum class State { DOWNLOADING_PACKAGE, EXTRACTING, ERROR };

 public:
  LoadingController(bool preview = false);

  void Update(std::chrono::microseconds ms) override;
  void DrawImGui() override;

 private:
  float progress_ = 0.0f;
  std::vector<std::byte> package_;
  std::vector<std::byte> file_buffer_;
  State state_ = State::DOWNLOADING_PACKAGE;
  mtar_t tar_;
};

}  // namespace player
}  // namespace ovis