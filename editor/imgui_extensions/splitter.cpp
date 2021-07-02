#include "splitter.hpp"

#include <imgui_internal.h>

namespace ImGui {

bool HorizontalSplitter(const char* label, float* top_height, float* bottom_height, float top_weight,
                        float bottom_weight, float height, float top_min_height, float bottom_min_height,
                        float thickness) {
  ImGuiContext& g = *GImGui;
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##Splitter");

  IM_ASSERT(top_weight > 0);
  IM_ASSERT(bottom_weight > 0);

  const float content_height = height > 0.0f ? height : GetContentRegionAvail().y;
  const float default_top_height = content_height * (top_weight / (top_weight + bottom_weight));
  const float default_bottom_height = content_height * (bottom_weight / (top_weight + bottom_weight));

  ImGuiStorage* storage = ImGui::GetStateStorage();

  ImGui::PushID(ImGui::GetID(label));
  auto top_window_height_id = ImGui::GetID("TopWindowSize");
  auto bottom_window_height_id = ImGui::GetID("BottomWindowSize");
  ImGui::PopID();

  *top_height = storage->GetFloat(top_window_height_id, default_top_height);
  *bottom_height = storage->GetFloat(bottom_window_height_id, default_bottom_height);

  ImRect bb;
  bb.Min.x = window->DC.CursorPos.x;
  bb.Min.y = window->DC.CursorPos.y + *top_height;

  const auto item_size = CalcItemSize(ImVec2(height, thickness), 0.0f, 0.0f);
  bb.Max.x = bb.Min.x + item_size.x;
  bb.Max.y = bb.Min.y + item_size.y;

  auto return_value =
      SplitterBehavior(bb, id, ImGuiAxis_Y, top_height, bottom_height, top_min_height, bottom_min_height, 0.0f);

  storage->SetFloat(top_window_height_id, *top_height);
  storage->SetFloat(bottom_window_height_id, *bottom_height);

  return return_value;
}

}  // namespace ImGui
