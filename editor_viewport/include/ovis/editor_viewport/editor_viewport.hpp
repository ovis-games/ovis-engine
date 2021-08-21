#pragma once

#include <emscripten/val.h>

#include <ovis/application/window.hpp>
#include <ovis/editor_viewport/camera_controller.hpp>
#include <ovis/editor_viewport/object_selection_controller.hpp>
#include <ovis/editor_viewport/render_passes/selected_object_bounding_box.hpp>

namespace ovis {
namespace editor {

class EditorViewport : public Window {
 public:
  EditorViewport();
  ~EditorViewport();

  void SetEventCallback(emscripten::val event_callback);
  void SendEvent(emscripten::val event);

  CameraController* camera_controller() { return &camera_controller_; }
  ObjectSelectionController* object_selection_controller() { return &object_selection_controller_; }

  // SelectedObjectBoundingBox* selected_object_bounding_box() { return &selected_object_bounding_box_; }

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

  static EditorViewport* instance() { return instance_; }

 private:
  static EditorViewport* instance_;

  std::vector<ViewportController*> controllers_;
  CameraController camera_controller_;
  ObjectSelectionController object_selection_controller_;

  // SelectedObjectBoundingBox selected_object_bounding_box_;

  emscripten::val event_callback_;

  void AddController(ViewportController* controller);
};

}
}
