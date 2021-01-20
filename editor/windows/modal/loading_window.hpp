#pragma once

#include <set>
#include <vector>
#include <string>

#include <emscripten/fetch.h>

#include "modal_window.hpp"

namespace ove {

class LoadingWindow : public ModalWindow {
 public:
  LoadingWindow();

 private:
  void DrawContent() override;

  std::vector<std::string> files_to_download_;
  std::set<std::string> downloaded_files_;
  std::string current_file_;

  static void FileListDownloadSucceded(emscripten_fetch_t* fetch);
  static void FileDownloadSucceded(emscripten_fetch_t* fetch);
  static void DownloadFailed(emscripten_fetch_t* fetch);
  void DownloadNextFile();
};

}  // namespace ove
