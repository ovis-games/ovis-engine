#include "editor_controller.hpp"

#include "../scene_view_editor.hpp"

namespace ovis::editor {

void EditorController::SubmitChanges() {
  SDL_assert(editor_ != nullptr);
  editor_->SubmitChanges();
}

}  // namespace ovis::editor
