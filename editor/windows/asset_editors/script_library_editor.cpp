#include "script_library_editor.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <ovis/core/scripting.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {
namespace editor {

ScriptLibraryEditor::ScriptLibraryEditor(const std::string& asset_id) : AssetEditor(asset_id) {
  SetupJsonFile({{"actions", json::array()}});
}

void ScriptLibraryEditor::DrawContent() {
  json serialized_chunk = GetCurrentJsonFileState();

  if (ImGui::Button("Run")) {
    if (chunk_.Deserialize(serialized_chunk)) {
      chunk_.Execute();
    }
  }

  bool submit_changes = false;
  auto& actions = serialized_chunk["actions"];
  for (auto& action : IndexRange(actions)) {
    DrawSpace(action.index());
    if (DrawAction(action.value(), action.index())) {
      submit_changes = true;
    }
  }
  DrawSpace(actions.size());
  if (submit_changes) {
    SubmitJsonFile(serialized_chunk, "json");
  }
}

bool ScriptLibraryEditor::DrawAction(json& action, size_t index) {
  bool submit_changes = false;

  ImGui::PushID("Action");
  ImGui::PushID(index);

  ImGui::Separator();
  ImGui::BeginGroup();
  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  ImGui::PushFont(font_awesome);
  if (ImGui::SmallButton("\uf7a4")) {
  }
  ImGui::PopFont();
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    ImGui::SetDragDropPayload("action", &index, sizeof(index));
    ImGui::BeginGroup();
    const std::string& action_name = action["function"];
    ImGui::Text("%s", action_name.c_str());
    ImGui::EndGroup();
    ImGui::EndDragDropSource();
  }


  const std::string& function_identifer = action["function"];
  const ScriptFunction* function = global_script_context.GetFunction(function_identifer);
  if (function == nullptr) {
    ImGui::Text("Function definition not found for: %s", function_identifer.c_str());
  } else {
    ImGui::Text("%s", function_identifer.c_str());
    ImGui::Text("Inputs: ");

    for (const auto& input : IndexRange(function->inputs)) {
      ImGui::PushID("Inputs");
      ImGui::PushID(input.index());

      ImGui::Text("%s : %s = ", input.value().identifier.c_str(), "Number");
      ImGui::SameLine();

      ImGui::PushItemWidth(100);
      std::string input_value = action["inputs"][std::to_string(input.index())];
      if (ImGui::InputText("###Value", &input_value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        action["inputs"][std::to_string(input.index())] = input_value;
        submit_changes = true;
      }
      ImGui::PopItemWidth();

      if (ImGui::BeginDragDropTarget()) {
        auto payload = ImGui::AcceptDragDropPayload("value");

        if (payload != nullptr) {
          const char* output_reference = reinterpret_cast<const char*>(payload->Data);
          action["inputs"][std::to_string(input.index())] = output_reference;
          submit_changes = true;
        }

        ImGui::EndDragDropTarget();
      }

      ImGui::PopID();
      ImGui::PopID();
    }
    ImGui::Text("Outputs: ");
    for (const auto& output : IndexRange(function->outputs)) {
      ImGui::PushID("Output");
      ImGui::PushID(output.index());

      const std::string button_label = fmt::format("{} : {}", output->identifier, "Number");
      ImGui::SmallButton(button_label.c_str());

      if (ImGui::BeginDragDropSource()) {
        const std::string output_reference = fmt::format("${}:{}", index, output->identifier);
        ImGui::SetDragDropPayload("value", output_reference.data(), output_reference.size() + 1);
        ImGui::SmallButton(button_label.c_str());
        ImGui::EndDragDropSource();
      }

      ImGui::PopID();
      ImGui::PopID();
    }
  }

  ImGui::EndGroup();

  ImGui::Separator();

  ImGui::PopID();
  ImGui::PopID();

  return submit_changes;
}

void ScriptLibraryEditor::DrawSpace(size_t target_index) {
  ImGui::PushItemWidth(-1);
  std::string function_identifier;
  const std::string label = fmt::format("###Space{}", target_index);
  if (ImGui::InputText(label.c_str(), &function_identifier, ImGuiInputTextFlags_EnterReturnsTrue)) {
    DoOnceAfterUpdate([this, target_index, function_identifier]() {
      auto function = global_script_context.GetFunction(function_identifier);
      if (function == nullptr) {
        return;
      }

      json action = {{"type", "function_call"}, { "function", function_identifier }, {"inputs", {}}};
      for (size_t i = 0; i < function->inputs.size(); ++i) {
        action["inputs"][std::to_string(i)] = "";
      }

      json serialized_chunk = GetCurrentJsonFileState();
      auto& actions = serialized_chunk["actions"];
      actions.insert(actions.begin() + target_index, action);
      SubmitJsonFile(serialized_chunk, "json");
    });
  }
  ImGui::PopItemWidth();

  if (ImGui::BeginDragDropTarget()) {
    auto payload = ImGui::AcceptDragDropPayload("action");

    if (payload != nullptr) {
      SDL_assert(payload->DataSize == sizeof(size_t));
      const size_t source_index = *reinterpret_cast<size_t*>(payload->Data);
      json serialized_chunk = GetCurrentJsonFileState();
      auto& actions = serialized_chunk["actions"];
      json action = actions[source_index];
      actions.erase(actions.begin() + source_index);
      actions.insert(actions.begin() + target_index - (target_index > source_index ? 1 : 0), std::move(action));
      SubmitJsonFile(serialized_chunk);
    }

    ImGui::EndDragDropTarget();
  }
}

}  // namespace editor
}  // namespace ovis
