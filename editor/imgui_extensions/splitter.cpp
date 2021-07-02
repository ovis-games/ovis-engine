#include "splitter.hpp"

#include <imgui_internal.h>

namespace ImGui {

bool HorizontalSplitter(const char* label, float* top_height, float* bottom_height, float top_weight,
                        float bottom_weight, ImVec2 size, float top_min_height, float bottom_min_height,
                        float thickness) {
  ImGuiContext& g = *GImGui;
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##Splitter");

  IM_ASSERT(top_weight > 0);
  IM_ASSERT(bottom_weight > 0);

  const auto available_content = GetContentRegionAvail();

  const float content_height = size.y > 0.0f ? size.y : available_content.y - size.y;
  const float default_top_height = content_height * (top_weight / (top_weight + bottom_weight)) - thickness * 0.5f;
  const float default_bottom_height = content_height * (bottom_weight / (top_weight + bottom_weight)) - thickness * 0.5f;

  const float content_width = size.x > 0.0f ? size.x : available_content.x - size.x;

  ImGuiStorage* storage = ImGui::GetStateStorage();

  ImGui::PushID(ImGui::GetID(label));
  auto top_window_height_id = ImGui::GetID("TopWindowSize");
  auto bottom_window_height_id = ImGui::GetID("BottomWindowSize");
  auto content_height_id = ImGui::GetID("ContentHeight");
  ImGui::PopID();

  *top_height = storage->GetFloat(top_window_height_id, default_top_height);
  *bottom_height = storage->GetFloat(bottom_window_height_id, default_bottom_height);
  const float previous_content_height = storage->GetFloat(content_height_id, content_height);
  storage->SetFloat(content_height_id, content_height);

  if (previous_content_height != content_height) {
    const float factor = content_height / previous_content_height;
    *top_height = std::round(*top_height * factor); // TODO: is the round necessary? are non-integer values okay for sizes?
    *bottom_height = content_height - thickness - *top_height;
  }

  ImRect bb;
  bb.Min.x = window->DC.CursorPos.x;
  bb.Min.y = window->DC.CursorPos.y + *top_height;

  const auto item_size = CalcItemSize(ImVec2(content_width, thickness), 0.0f, 0.0f);
  bb.Max.x = bb.Min.x + item_size.x;
  bb.Max.y = bb.Min.y + item_size.y;

  auto return_value =
      SplitterBehavior(bb, id, ImGuiAxis_Y, top_height, bottom_height, top_min_height, bottom_min_height, 0.0f);

  storage->SetFloat(top_window_height_id, *top_height);
  storage->SetFloat(bottom_window_height_id, *bottom_height);

  return return_value;
}

}  // namespace ImGui
