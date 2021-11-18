#include <ovis/utils/log.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/imgui/imgui_end_frame_controller.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {

ImGuiEndFrameController::ImGuiEndFrameController() : SceneController(Name()) {
  UpdateAfter<ImGuiStartFrameController>();

  cursors_[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  cursors_[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  cursors_[ImGuiMouseCursor_None] = nullptr;
  cursors_[ImGuiMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
  cursors_[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  cursors_[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  cursors_[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
  cursors_[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  cursors_[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  cursors_[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
}

void ImGuiEndFrameController::Update(std::chrono::microseconds delta_time) {
  auto start_frame_controller = scene()->GetController<ImGuiStartFrameController>();
#ifdef _DEBUG
  if (!start_frame_controller) {
    LogE("Controller '{}' needs controller '{}' to be present", Id(), ImGuiStartFrameController::Id());
  }
  SDL_assert(ImGui::GetCurrentContext() == start_frame_controller->imgui_context_.get());
#endif
  if (start_frame_controller != nullptr && start_frame_controller->frame_started_) {
    ImGui::EndFrame();
    SDL_SetCursor(cursors_[ImGui::GetMouseCursor()]);
  }
}

}  // namespace ovis
