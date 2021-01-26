#pragma once

#include <ovis/engine/scene_controller.hpp>

namespace ovis {
namespace player {

class LoadingController : public ovis::SceneController {
 public:
  LoadingController(bool preview = false);

  void DrawImGui() override;

 private:
  float progress_ = 0.3f;
};

}  // namespace player
}  // namespace ovis