#pragma once

#include <set>

#include <emscripten/fetch.h>

#include <ovis/engine/scene_controller.hpp>

namespace ove {

class LoadingController : public ovis::SceneController {
 public:
  LoadingController();

  // void Update(std::chrono::microseconds delta_time) override;
  void DrawImGui() override;

  inline bool is_finished() const { return finished_; }

 private:
  bool finished_ = false;
  std::vector<std::string> files_to_download_;
  std::set<std::string> downloaded_files_;
  std::string current_file_;

  static void FileListDownloadSucceded(emscripten_fetch_t* fetch);
  static void FileDownloadSucceded(emscripten_fetch_t* fetch);
  static void DownloadFailed(emscripten_fetch_t* fetch);
  void DownloadNextFile();
};

}  // namespace ove
