#pragma once

#include "ui_window.hpp"

namespace ovis {
namespace editor {

class LogWindow : public UiWindow {
 public:
  LogWindow();

 protected:
  void DrawContent() override;

 private:
  bool show_verbose_ = false;
  bool show_debug_ = false;
  bool show_info_ = true;
  bool show_warning_ = true;
  bool show_error_ = true;
  bool auto_scrolling_ = true;
  const std::vector<std::string>* log_history_;
  std::size_t filtered_entries_count_ = 0;
  bool scroll_next_frame_ = false;
};

}  // namespace editor
}  // namespace ovis