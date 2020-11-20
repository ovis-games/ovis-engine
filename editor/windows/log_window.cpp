#include "log_window.hpp"

#include <imgui.h>

#include <ovis/core/log.hpp>

namespace ove {

LogWindow::LogWindow(const std::vector<std::string>* log_history) : log_history_(log_history) {}

void LogWindow::Draw() {
  static bool x = true;
  if (ImGui::Begin("Log", &x, ImGuiWindowFlags_HorizontalScrollbar)) {
    ImGui::Checkbox("Verbose", &show_verbose_);
    ImGui::SameLine();
    ImGui::Checkbox("Debug", &show_debug_);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &show_info_);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &show_warning_);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &show_error_);
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20.0f, 0.0f));
    ImGui::SameLine();
    ImGui::Checkbox("Auto Scrolling", &auto_scrolling_);

    ImGui::BeginChild("LogText", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);

    size_t filtered_entries_count = 0;
    for (const auto& text : *log_history_) {
      const char category = text[0];

      switch (category) {
        case 'V':
          if (show_verbose_) {
            ImGui::TextColored(ImVec4(0.0, 1.0, 1.0, 1.0), "%s", text.c_str());
            ++filtered_entries_count;
          }
          break;
        case 'D':
          if (show_debug_) {
            ImGui::TextColored(ImVec4(1.0, 0.5, 0.0, 1.0), "%s", text.c_str());
            ++filtered_entries_count;
          }
          break;
        case 'I':
          if (show_info_) {
            ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "%s", text.c_str());
            ++filtered_entries_count;
          }
          break;
        case 'W':
          if (show_warning_) {
            ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "%s", text.c_str());
            ++filtered_entries_count;
          }
          break;
        case 'E':
          if (show_error_) {
            ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "%s", text.c_str());
            ++filtered_entries_count;
          }
          break;
        default:
          ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "%s", text.c_str());
          ++filtered_entries_count;
      }
    }

    if (scroll_next_frame_) {
      ImGui::SetScrollHere(1.0f);
      scroll_next_frame_ = false;
    }
    if (filtered_entries_count != filtered_entries_count_ && auto_scrolling_) {
      scroll_next_frame_ = true;
      filtered_entries_count_ = filtered_entries_count;
    }
    ImGui::EndChild();
  }
  ImGui::End();
}

}  // namespace ove