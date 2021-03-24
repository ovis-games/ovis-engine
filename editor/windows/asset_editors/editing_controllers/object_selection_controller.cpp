#include "object_selection_controller.hpp"

#include <ovis/utils/log.hpp>

namespace ovis {
namespace editor {

ObjectSelectionController::ObjectSelectionController(Scene* game_scene)
    : SceneController("ObjectSelectionController"), game_scene_(game_scene) {}

void ObjectSelectionController::ProcessEvent(Event* event) {}

void ObjectSelectionController::SelectObject(const std::string& object_name) {
  LogV("Selected object: {}", object_name);
  selected_object_name_ = object_name;
  CheckSelectionValidity();
}
void ObjectSelectionController::SelectObject(SceneObject* object) {
  if (object != nullptr) {
    SDL_assert(object->scene() == game_scene_);
    SelectObject(object->name());
  } else {
    selected_object_name_ = "";
  }
}
void ObjectSelectionController::ClearSelection() {
  LogV("Cleared selection");
  selected_object_name_ = "";
}

bool ObjectSelectionController::has_selected_object() const {
  CheckSelectionValidity();
  return selected_object_name_ != "";
}
const std::string& ObjectSelectionController::selected_object_name() const {
  CheckSelectionValidity();
  return selected_object_name_;
}
SceneObject* ObjectSelectionController::selected_object() const {
  CheckSelectionValidity();
  return selected_object_name_.length() > 0 ? game_scene_->GetObject(selected_object_name_) : nullptr;
}

void ObjectSelectionController::CheckSelectionValidity() const {
  if (selected_object_name_ != "" && !game_scene_->ContainsObject(selected_object_name_)) {
    LogV("The game scene does not contain the object {}", selected_object_name_);
    selected_object_name_ = "";
  }
}

}  // namespace editor
}  // namespace ovis
