#pragma once

#include <vector>

#include <ovis/imgui/imgui_window.hpp>

namespace ovis {
namespace editor {

enum class ToastType { INFO, WARNING, ERROR };

class ToastWindow : public ImGuiWindow {
 public:
  ToastWindow();

  void AddNotification(ToastType type, std::string_view message);

 private:
  void DrawContent() override;
  void BeforeBegin() override;

  struct Notification {
    ToastType type;
    std::string message;
    double creation_time;
    double fade_start_time;
  };
  std::vector<Notification> notifications_;
  double current_time_ = 0.0;
  double display_time_ = 5.0;
  double fading_time_ = 3.0;
};

}  // namespace editor
}  // namespace ovis
