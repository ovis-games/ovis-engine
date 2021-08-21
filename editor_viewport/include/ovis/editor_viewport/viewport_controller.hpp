#pragma once

#include <chrono>
#include <ovis/core/event.hpp>

namespace ovis {
namespace editor {

class ViewportController {
 public:
  virtual void Update(std::chrono::microseconds delta_time) {}
  virtual void ProcessEvent(Event* event) {}
};

}  // namespace editor
}  // namespace ovis
