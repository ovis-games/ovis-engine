#pragma once

#include <chrono>
#include "ovis/core/scene_controller.hpp"
#include <ovis/core/event.hpp>

namespace ovis {
namespace editor {

class EditorViewport;

class ViewportController {
  friend class EditorViewport;

 public:
  ViewportController(EditorViewport* editor_viewport) : editor_viewport_(editor_viewport) {}

  virtual void Update(std::chrono::microseconds delta_time) {}
  virtual void ProcessEvent(Event* event) {}

  EditorViewport* editor_viewport() const { return editor_viewport_; }

 private:
  EditorViewport* editor_viewport_;
};

}  // namespace editor
}  // namespace ovis
