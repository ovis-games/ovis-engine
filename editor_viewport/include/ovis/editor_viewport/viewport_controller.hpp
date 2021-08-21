#pragma once

#include <chrono>
#include <ovis/core/event.hpp>

namespace ovis {
namespace editor {

class EditorViewport;

class ViewportController {
  friend class EditorViewport;

 public:
  virtual void Update(std::chrono::microseconds delta_time) {}
  virtual void ProcessEvent(Event* event) {}

  EditorViewport* viewport() const;

 private:
};

}  // namespace editor
}  // namespace ovis
