
#include "toast_window.hpp"

#include <imgui.h>

#include <ovis/utils/log.hpp>
#include <ovis/utils/range.hpp>

namespace ovis {
namespace editor {

ToastWindow::ToastWindow() : ImGuiWindow("ToastWindow") {
  UpdateAfter("Overlay");
  SetFlags(ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
           ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground);
}

void ToastWindow::AddNotification(ToastType type, std::string_view message) {
  notifications_.push_back({type, std::string(message), current_time_, std::numeric_limits<float>::quiet_NaN()});
}

void ToastWindow::DrawContent() {
  auto GetNotificationColor = [](ToastType type) {
    switch (type) {
      case ToastType::INFO:
        return ImVec4(50.0f / 255.0f, 121.0f / 255.0f, 174.0f / 255.0f, 1.0f);
      case ToastType::WARNING:
        return ImVec4(255.0f / 255.0f, 184.0f / 255.0f, 5.0f / 255.0f, 1.0f);
      case ToastType::ERROR:
        return ImVec4(111.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);
    }
  };

  current_time_ = ImGui::GetTime();

  notifications_.erase(std::remove_if(notifications_.begin(), notifications_.end(),
                                      [this](const Notification& notification) {
                                        return (current_time_ - notification.creation_time) >= display_time_ &&
                                               (current_time_ - notification.fade_start_time) >= fading_time_;
                                      }),
                       notifications_.end());

  std::vector<size_t> notifications_to_delete;
  notifications_to_delete.reserve(notifications_.size());

  const ImVec2 main_viewport_size = ImGui::GetMainViewport()->Size;

  for (const auto& notification : IndexRange(notifications_)) {
    const std::string id = std::to_string(notification.index());

    float alpha = 1.0f;
    if ((current_time_ - notification->creation_time) >= display_time_) {
      if (std::isnan(notification->fade_start_time)) {
        notification->fade_start_time = current_time_;
      }
      alpha = 1.0f - ((current_time_ - notification->fade_start_time) / fading_time_);
    } else {
      alpha = std::min<float>(1.0f, (current_time_ - notification->creation_time) / 0.5f);
    }

    const ImVec4 text_color(1.0f, 1.0f, 1.0f, alpha);

    ImGui::SetNextWindowBgAlpha(alpha);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GetNotificationColor(notification->type));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 0.0f));
    if (ImGui::BeginChild(id.c_str(),
                          ImVec2(std::min(main_viewport_size.x, 600.0f), ImGui::GetTextLineHeightWithSpacing()), false,
                          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysUseWindowPadding)) {
      ImGui::TextColored(text_color, "%s", notification->message.c_str());
      if (ImGui::IsWindowHovered()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
          notifications_to_delete.push_back(notification.index());
        } else {
          notification->fade_start_time = current_time_;
        }
      }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndChild();
  }

  // We need to remove from back to front, otherwise the indices will change. So sort the vector descending.
  std::sort(notifications_to_delete.begin(), notifications_to_delete.end(),
            [](auto lhs, auto rhs) { return lhs >= rhs; });
  for (size_t notification_index : notifications_to_delete) {
    SDL_assert(notification_index < notifications_.size());
    notifications_.erase(notifications_.begin() + notification_index);
  }
}

void ToastWindow::BeforeBegin() {
  SDL_assert(ImGui::GetMainViewport() != nullptr);
  const ImVec2 main_viewport_size = ImGui::GetMainViewport()->Size;
  const ImVec2 window_position = {main_viewport_size.x * 0.5f, 0.0f};
  ImGui::SetNextWindowPos(window_position, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
}

}  // namespace editor
}  // namespace ovis
