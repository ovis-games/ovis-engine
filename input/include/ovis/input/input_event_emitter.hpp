#pragma once

#include "ovis/core/event_storage.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/input/key_events.hpp"
#include "ovis/input/mouse_events.hpp"

namespace ovis {

// This is a simple job that stores all kinds of input events and emits them
// when it is executed. Its intended use is for situations where the input events are
// passed to the application ouside of a frame job.
class InputEventEmitter : public FrameJob {
 public:
  InputEventEmitter();

  Result<> Prepare(Scene* const& scene);
  Result<> Execute(const SceneUpdate& update);

  void PushEvent(KeyPressEvent event) { key_press_events_.push_back(std::move(event)); }
  void PushEvent(KeyReleaseEvent event) { key_release_events_.push_back(std::move(event)); }
  void PushEvent(MouseButtonPressEvent event) { mouse_button_press_events_.push_back(std::move(event)); }
  void PushEvent(MouseButtonReleaseEvent event) { mouse_button_release_events_.push_back(std::move(event)); }
  void PushEvent(MouseMoveEvent event) { mouse_move_events_.push_back(std::move(event)); }
  void PushEvent(MouseWheelEvent event) { mouse_wheel_events_.push_back(std::move(event)); }

 private:
  std::vector<KeyPressEvent> key_press_events_;
  std::vector<KeyReleaseEvent> key_release_events_;
  std::vector<MouseButtonPressEvent> mouse_button_press_events_;
  std::vector<MouseButtonReleaseEvent> mouse_button_release_events_;
  std::vector<MouseMoveEvent> mouse_move_events_;
  std::vector<MouseWheelEvent> mouse_wheel_events_;

  EventEmitter<KeyPressEvent> key_press_event_emitter_;
  EventEmitter<KeyReleaseEvent> key_release_event_emitter_;
  EventEmitter<MouseButtonPressEvent> mouse_button_press_event_emitter_;
  EventEmitter<MouseButtonReleaseEvent> mouse_button_release_event_emitter_;
  EventEmitter<MouseMoveEvent> mouse_move_event_emitter_;
  EventEmitter<MouseWheelEvent> mouse_wheel_event_emitter_;
};

}  // namespace ovis
