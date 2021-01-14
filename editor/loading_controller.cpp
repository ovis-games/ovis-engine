#include "loading_controller.hpp"

#include <imgui.h>

#include "global.hpp"

#include <ovis/core/json.hpp>
#include <ovis/core/log.hpp>
#include <filesystem>
#include <fstream>

namespace ove {

LoadingController::LoadingController() : ovis::SceneController("LoadingController") {
  if (!std::filesystem::exists("/assets")) {
    std::filesystem::create_directory("/assets");
  }

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = &LoadingController::FileListDownloadSucceded;
  attr.onerror = &LoadingController::DownloadFailed;
  attr.userData = this;
  attr.withCredentials = true;

  const std::string url = backend_url + "/v1/games/" + project_id + "/assetFiles";
  emscripten_fetch(&attr, url.c_str());
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
  ImGui::Begin("Ovis Editor", nullptr, window_flags);
  ImGui::PopStyleVar();

  ImGui::PopStyleVar(2);

  const float progress = downloaded_files_.size() / static_cast<float>(files_to_download_.size());
  ImGui::ProgressBar(progress);

  ImGui::End();
}

void LoadingController::FileListDownloadSucceded(emscripten_fetch_t* fetch) {
  std::string_view data(fetch->data, fetch->numBytes);
  try {
    ovis::json files = ovis::json::parse(data);

    auto controller = reinterpret_cast<LoadingController*>(fetch->userData);
    controller->files_to_download_ = files.get<std::vector<std::string>>();
    controller->DownloadNextFile();
  } catch (const ovis::json::parse_error& error) {
    ovis::LogE("Invalid json: {}", error.what());
  }
}

void LoadingController::FileDownloadSucceded(emscripten_fetch_t* fetch) {
  auto controller = reinterpret_cast<LoadingController*>(fetch->userData);
  ovis::LogD("Successfully downloaded '{}'", controller->current_file_);

  std::ofstream file(fmt::format("/assets/{}", controller->current_file_));
  SDL_assert(file.is_open());
  file.write(fetch->data, fetch->numBytes);
  file.close();

  controller->DownloadNextFile();
}

void LoadingController::DownloadFailed(emscripten_fetch_t* fetch) {
  const std::string error(fetch->data, fetch->data + fetch->numBytes);
  ovis::LogE("Failed to download {}:\n{}", fetch->url, error);
}

void LoadingController::DownloadNextFile() {
  if (current_file_ != "") {
    downloaded_files_.insert(current_file_);
  }
  if (files_to_download_.size() > 0) {
    current_file_ = files_to_download_.back();
    files_to_download_.pop_back();

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = &LoadingController::FileDownloadSucceded;
    attr.onerror = &LoadingController::DownloadFailed;
    attr.userData = this;
    attr.withCredentials = true;

    const std::string url = backend_url + "/v1/games/" + project_id + "/assetFiles/" + current_file_;
    emscripten_fetch(&attr, url.c_str());

    ovis::LogD("Downloading '{}'", current_file_);
  } else {
    current_file_.clear();
    ovis::LogD("No more files to download");
    finished_ = true;
  }
}

}  // namespace ove
