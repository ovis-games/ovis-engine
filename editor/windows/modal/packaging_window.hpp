#pragma once

#include <set>
#include <vector>
#include <string>

#include <emscripten/fetch.h>

#include "modal_window.hpp"

namespace ove {

class PackagingWindow : public ModalWindow {
 public:
  PackagingWindow();

 private:
  void DrawContent() override;
  void DrawConfiguration();
  void DrawProgress();

  std::string version_ = "preview";
  bool is_packaging_ = false;
  float progress_ = 0.0f;
};

}  // namespace ove
