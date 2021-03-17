#pragma once

#include <ovis/scene/scene_controller.hpp>

namespace ovis {

class TransformController : public SceneController {
 public:
  TransformController();

  void Update(std::chrono::microseconds delta_time) override;

  static void RegisterType(sol::table* module);
};

}  // namespace ovis