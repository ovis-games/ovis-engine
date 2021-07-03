#include "script_library_editor.hpp"
#include "imgui.h"
#include <ovis/core/scripting.hpp>

namespace ovis {
namespace editor {

ScriptLibraryEditor::ScriptLibraryEditor(const std::string& asset_id) : AssetEditor(asset_id) {
  actions_ = {
    {{"function", "add"}},
    {{"function", "negate"}}
  };
}

void ScriptLibraryEditor::DrawContent() {
  for (auto& action : IndexRange(actions_)) {
    DrawSpace(action.index());
    DrawAction(action.value(), action.index());
  }
  DrawSpace(actions_.size());
}

void ScriptLibraryEditor::DrawAction(json& action, size_t index) {
  ImGui::BeginGroup();
  const std::string& function_identifer = action["function"];
  const ScriptFunction* function = global_script_context.GetFunction(function_identifer);
  if (function == nullptr) {
    ImGui::Text("Function definition not found for: %s", function_identifer.c_str());
  } else {
    ImGui::Text("%s", function_identifer.c_str());
    ImGui::Text("Inputs: ");
    for (const auto& input : function->inputs) {
      ImGui::Text("[type]");
      ImGui::SameLine();
      ImGui::SmallButton("name");
    }
    ImGui::Text("Outputs: ");
    for (const auto& output : function->outputs) {
      ImGui::Text("[type]");
      ImGui::SameLine();
      ImGui::SmallButton("name");
    }
  }
  ImGui::EndGroup();


  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    ImGui::SetDragDropPayload("action", &index, sizeof(index));
    ImGui::BeginGroup();
    const std::string& action_name = action["function"];
    ImGui::Text("%s", action_name.c_str());
    ImGui::EndGroup();
    ImGui::EndDragDropSource();
  }
}

void ScriptLibraryEditor::DrawSpace(size_t target_index) {
  ImGui::PushID("DropSpace");
  ImGui::PushID(target_index);
  ImGui::InvisibleButton("Button", ImVec2(200, 5));
  if (ImGui::BeginDragDropTarget()) {
    auto payload = ImGui::AcceptDragDropPayload("action");

    if (payload != nullptr) {
      SDL_assert(payload->DataSize == sizeof(size_t));
      const size_t source_index = *reinterpret_cast<size_t*>(payload->Data);
      json action = actions_[source_index];
      actions_.erase(actions_.begin() + source_index);
      actions_.insert(actions_.begin() + target_index - (target_index > source_index ? 1 : 0), std::move(action));
    }

    ImGui::EndDragDropTarget();
  }
  ImGui::PopID();
  ImGui::PopID();
}

}  // namespace editor
}  // namespace ovis
