
#include "message_box_window.hpp"

#include <imgui.h>

#include <ovis/utils/log.hpp>
#include <ovis/utils/range.hpp>

namespace ovis {
namespace editor {

MessageBoxWindow::MessageBoxWindow(const std::string& id, std::string_view window_title, std::string_view message,
                                   std::span<std::string_view> buttons,
                                   std::function<void(const std::string& pressed_button)> callback)
    : ModalWindow(id, std::string(window_title)),
      message_(message),
      buttons_(buttons.begin(), buttons.end()),
      callback_(std::move(callback)) {}

void MessageBoxWindow::DrawContent() {
  ImGui::Text("%s", message_.c_str());

  for (const auto& button : IndexRange(buttons_)) {
    if (button.index() > 0) {
      ImGui::SameLine();
    }
    if (ImGui::Button(button->c_str())) {
      callback_(*button);
      Remove();
    }
  }
}

}  // namespace editor
}  // namespace ovis
