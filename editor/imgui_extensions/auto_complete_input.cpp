#include "auto_complete_input.hpp"

#include "imgui.h"

#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <ovis/utils/log.hpp>

namespace ImGui {

namespace {

struct AutoCompleteState {
  ImGuiStorage* storage;
  ImGuiWindow* window;
  std::span<const std::string> auto_complete_values;
  bool filter_changed;
};

int InputCallback(ImGuiInputTextCallbackData* data) {
  AutoCompleteState* state = reinterpret_cast<AutoCompleteState*>(data->UserData);
  if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
    state->filter_changed = true;
  }
  return 0;
}

}  // namespace

bool AutoCompleteInput(const char* label, std::string* value, const std::string* predefined_values, int value_count,
                       int* selected_index, bool* is_active_output, int flags) {
  // TODO: See implementation at https://gist.github.com/harold-b/7dcc02557c2b15d76c61fde1186e31d0
  bool value_changed = false;

  AutoCompleteState state{
      .storage = GetStateStorage(),
      .window = GetCurrentWindow(),
      .auto_complete_values = {predefined_values, static_cast<std::size_t>(value_count)},
      .filter_changed = false,
  };

  const auto storage = ImGui::GetStateStorage();
  const auto was_active_last_frame_id = ImGui::GetID("was_active_last_frame");
  const bool was_active_last_frame = storage->GetBool(was_active_last_frame_id, true);
  const auto focus_next_frame_id = ImGui::GetID("focus_next_frame");
  const bool focus_next_frame = storage->GetBool(focus_next_frame_id, false);
  storage->SetBool(focus_next_frame_id, false);

  ImGui::PushID(label);
  if (focus_next_frame) {
    SetKeyboardFocusHere();
  }
  ImGui::InputText(label, value, ImGuiInputTextFlags_CallbackEdit, &InputCallback, &state);
  // if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
  // !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))) {
  //   ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
  // }

  ImGuiWindow* window = GetCurrentWindow();

  bool is_active = ImGui::IsItemActive() || ImGui::IsItemDeactivated();
  bool is_ = IsItemActivated();

  if (is_active || was_active_last_frame) {
    ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
    ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x, 200}, ImGuiCond_Appearing);

    if (ImGui::Begin("##autocomplete", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_Tooltip |
                         ImGuiWindowFlags_NoNav)) {
      ImGui::PushAllowKeyboardFocus(false);
      BringWindowToDisplayFront(GetCurrentWindow());
      if (IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        is_active = true;
        if (!IsMouseDown(ImGuiMouseButton_Left)) {
          storage->SetBool(focus_next_frame_id, true);
        }
      }

      std::string item_label;
      for (int i = 0; i < value_count; ++i) {
        if (std::strstr(predefined_values[i].c_str(), value->c_str())) {
          item_label = predefined_values[i];
          // item_label += "###";
          // item_label += std::to_string(i);

          if (ImGui::Selectable(item_label.c_str())) {
            *value = predefined_values[i];
            if (selected_index != nullptr) {
              *selected_index = i;
            }
            value_changed = true;
            is_active = false;
          }
        }
      }
      ImGui::PopAllowKeyboardFocus();
    }
    ImGui::End();
  }
  storage->SetBool(was_active_last_frame_id, is_active);
  ImGui::PopID();

  if (is_active_output) {
    *is_active_output = is_active || was_active_last_frame;
  }

  return value_changed;
}

}  // namespace ImGui
