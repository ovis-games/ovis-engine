#include <filesystem>

#include <imgui.h>
#include <ovis/player/loading_controller.hpp>

#include <ovis/core/log.hpp>
#include <ovis/engine/fetch.hpp>

namespace ovis {
namespace player {

LoadingController::LoadingController(bool preview) : SceneController("LoadingController") {
  if (!std::filesystem::exists("/assets")) {
    std::filesystem::create_directory("/assets");
  }
  if (!std::filesystem::exists("/tmp")) {
    std::filesystem::create_directory("/tmp");
  }

  std::string url = "/api/v1/games/a/packages/release";

  FetchOptions options;
  options.method = RequestMethod::GET;
  options.on_success = [this]() { LogI("Downloaded package!"); Remove(); };
  options.on_error = [this]() { LogE("Failed to download package!"); };
  options.on_progress = [this](const FetchProgress& progress) { LogI("{}/{}", progress.num_bytes, progress.total_bytes); };

  Fetch(url, options);
}

void LoadingController::DrawImGui() {
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  window_flags |=
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Ovis Player", nullptr, window_flags);
  ImGui::PopStyleVar();

  ImGui::PopStyleVar(2);

  ImGui::ProgressBar(progress_);

  ImGui::End();
}

}  // namespace player

}  // namespace ovis
