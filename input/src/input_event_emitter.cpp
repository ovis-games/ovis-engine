#include "ovis/input/input_event_emitter.hpp"

namespace ovis {


InputEventEmitter::InputEventEmitter() : FrameJob("InputEventEmitter") {
  RequireWriteAccess<KeyPressEvent>();
  RequireWriteAccess<KeyReleaseEvent>();
  RequireWriteAccess<MouseButtonPressEvent>();
  RequireWriteAccess<MouseButtonReleaseEvent>();
  RequireWriteAccess<MouseMoveEvent>();
  RequireWriteAccess<MouseWheelEvent>();
}

Result<> InputEventEmitter::Prepare(Scene* const& scene) {
  key_press_event_emitter_ = scene->GetEventEmitter<KeyPressEvent>();
  key_release_event_emitter_ = scene->GetEventEmitter<KeyReleaseEvent>();
  mouse_button_press_event_emitter_ = scene->GetEventEmitter<MouseButtonPressEvent>();
  mouse_button_release_event_emitter_ = scene->GetEventEmitter<MouseButtonReleaseEvent>();
  mouse_move_event_emitter_ = scene->GetEventEmitter<MouseMoveEvent>();
  mouse_wheel_event_emitter_ = scene->GetEventEmitter<MouseWheelEvent>();
}

Result<> InputEventEmitter::Execute(const SceneUpdate& update) {
  for (const auto& event : key_press_events_) {
    key_press_event_emitter_.Emit(event);
  }
  key_press_events_.clear();

  for (const auto& event : key_release_events_) {
    key_release_event_emitter_.Emit(event);
  }
  key_release_events_.clear();

  for (const auto& event : mouse_button_press_events_) {
    mouse_button_press_event_emitter_.Emit(event);
  }
  mouse_button_press_events_.clear();

  for (const auto& event : mouse_button_release_events_) {
    mouse_button_release_event_emitter_.Emit(event);
  }
  mouse_button_release_events_.clear();

  for (const auto& event : mouse_move_events_) {
    mouse_move_event_emitter_.Emit(event);
  }
  mouse_move_events_.clear();

  for (const auto& event : mouse_wheel_events_) {
    mouse_wheel_event_emitter_.Emit(event);
  }
  mouse_wheel_events_.clear();
}

}  // namespace ovis
