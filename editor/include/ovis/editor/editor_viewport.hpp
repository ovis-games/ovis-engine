#pragma once

#include "emscripten/val.h"

#include "ovis/utils/utf8.hpp"  // For GetComponentPath()
#include "ovis/emscripten/canvas.hpp"

namespace ovis {
namespace editor {

class EditorViewport : public Canvas {
 public:
  EditorViewport(std::string target);

  // CameraController* camera_controller() { return &camera_controller_; }
  // ObjectSelectionController* object_selection_controller() { return &object_selection_controller_; }
  // TransformationToolsController* transformation_tools_controller() { return &transformation_tools_controller_; }

  // // SelectedObjectBoundingBox* selected_object_bounding_box() { return &selected_object_bounding_box_; }

  // void Update(std::chrono::microseconds delta_time) override;
  // void ProcessEvent(Event* event) override;

  Scene* scene() { return &scene_; }

 private:
  Scene scene_;
};

emscripten::val GetDocumentValueAtPath(std::string_view path);

inline std::string GetComponentPath(std::string_view object_path, std::string_view component_id) {
  return fmt::format("/objects/{}/components/{}", replace_all(object_path, "/", "/children/"), component_id);
}

}
}
