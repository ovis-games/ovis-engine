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

  struct Version {
    int major;
    int minor;
    int patch;
  } version_ = { };
};

}  // namespace ove
