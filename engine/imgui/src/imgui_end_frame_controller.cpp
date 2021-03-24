#include <ovis/utils/log.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/imgui/imgui_end_frame_controller.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {

ImGuiEndFrameController::ImGuiEndFrameController() : SceneController(Name()) {
  UpdateAfter<ImGuiStartFrameController>();
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
  }
}

}  // namespace ovis
