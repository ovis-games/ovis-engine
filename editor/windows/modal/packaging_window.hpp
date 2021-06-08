#pragma once

#include "modal_window.hpp"
#include <set>
#include <string>
#include <vector>

namespace ovis {
namespace editor {

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

}  // namespace editor
}  // namespace ovis
