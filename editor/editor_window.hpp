#pragma once

#include "windows/toast_window.hpp"

#include <ovis/application/window.hpp>

namespace ovis {
namespace editor {

class EditorWindow : public Window {
 public:
  EditorWindow();

  void LoadGameWithId(const std::string& game_id);

  void Update(std::chrono::microseconds delta_time) override;

  static inline EditorWindow* instance() { return instance_; }

  inline void AddToastNotification(ToastType type, std::string_view message) {
    scene()->GetController<ToastWindow>("ToastWindow")->AddNotification(type, message);
  }

  template <typename... Args>
  inline void AddToastNotification(ToastType type, std::string_view message, Args&&... args) {
    const std::string formatted_message = fmt::format(message, std::forward<Args>(args)...);
    scene()->GetController<ToastWindow>("ToastWindow")->AddNotification(type, formatted_message);
  }

  void ShowMessageBox(std::string_view title, std::string_view message, std::vector<std::string_view> buttons,
                      std::function<void(std::string_view)> callback);

 private:
  static EditorWindow* instance_;

  void SetUIStyle();
};

}  // namespace editor
}  // namespace ovis