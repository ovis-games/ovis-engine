#include "editor_controller.hpp"

#include "../scene_view_editor.hpp"

namespace ovis::editor {

void EditorController::SubmitChangesToScene() {
  SDL_assert(editor_ != nullptr);
  editor_->SubmitChangesToScene();
}

}  // namespace ovis::editor
