#pragma once

#include <ovis/engine/scene_controller.hpp>

namespace ove
{
  
class ModalWindow : public ovis::SceneController {
public:
  ModalWindow(const std::string& controller_name, const std::string& window_title);

  void DrawImGui() override;

protected:
  virtual void DrawContent() {}

  std::string window_name_;
};

} // namespace ove
