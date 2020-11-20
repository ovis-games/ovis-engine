#pragma once

#include <ovis/engine/window.hpp>

namespace ove {

class EditorWindow : public ovis::Window {
 public:
  EditorWindow();

  void Update(std::chrono::microseconds delta_time) override;

  static inline EditorWindow* instance() { return instance_; }

 private:
  static EditorWindow* instance_;
};

}  // namespace ove