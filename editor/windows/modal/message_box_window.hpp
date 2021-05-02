#pragma once

#include "modal_window.hpp"
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ovis {
namespace editor {

class MessageBoxWindow : public ModalWindow {
 public:
  MessageBoxWindow(const std::string& id, std::string_view window_title, std::string_view message,
                   std::span<std::string_view> buttons,
                   std::function<void(const std::string& pressed_button)> callback);

 private:
  void DrawContent() override;

  std::function<void(const std::string& pressed_button)> callback_;
  std::string message_;
  std::vector<std::string> buttons_;
};

}  // namespace editor
}  // namespace ovis
