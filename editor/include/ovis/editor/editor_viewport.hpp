#pragma once

#include <emscripten/val.h>

#include "ovis/utils/utf8.hpp"  // For GetComponentPath()
#include "ovis/application/canvas_viewport.hpp"
#include "ovis/application/sdl_window.hpp"
#include "ovis/application/tick_receiver.hpp"
#include "ovis/editor/camera_controller.hpp"
#include "ovis/editor/object_selection_controller.hpp"
#include "ovis/editor/render_passes/selected_object_bounding_box.hpp"
#include "ovis/editor/transformation_tools_controller.hpp"
#include "ovis/editor/viewport_controller.hpp"

namespace ovis {
namespace editor {

class EditorViewport : public CanvasViewport, public TickReceiver {
 public:
  EditorViewport(std::string target);

  CameraController* camera_controller() { return &camera_controller_; }
  ObjectSelectionController* object_selection_controller() { return &object_selection_controller_; }
  TransformationToolsController* transformation_tools_controller() { return &transformation_tools_controller_; }

  // SelectedObjectBoundingBox* selected_object_bounding_box() { return &selected_object_bounding_box_; }

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  Scene scene_;

  std::vector<ViewportController*> controllers_;
  CameraController camera_controller_;
  ObjectSelectionController object_selection_controller_;
  TransformationToolsController transformation_tools_controller_;

  // SelectedObjectBoundingBox selected_object_bounding_box_;

  void AddController(ViewportController* controller);
};

emscripten::val GetDocumentValueAtPath(std::string_view path);

inline std::string GetComponentPath(std::string_view object_path, std::string_view component_id) {
  return fmt::format("/objects/{}/components/{}", replace_all(object_path, "/", "/children/"), component_id);
}

}
}
