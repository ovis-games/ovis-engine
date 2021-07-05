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
  editing_copy_ = GetCurrentJsonFileState();

  if (ImGui::Button("Run")) {
    if (chunk_.Deserialize(editing_copy_)) {
      chunk_.Execute();
    }
  }

  if (DrawActions(json::json_pointer("/actions"))) {
    SubmitJsonFile(editing_copy_, "json");
  }
}

bool ScriptLibraryEditor::DrawActions(json::json_pointer path) {
  auto& actions = editing_copy_[path];

  bool submit_changes = false;
  for (auto& action : IndexRange(actions)) {
    DrawSpace(path / action.index());
    if (DrawAction(path / action.index())) {
      submit_changes = true;
    }
  }
  DrawSpace(path / actions.size());
  return submit_changes;
}

bool ScriptLibraryEditor::DrawAction(json::json_pointer path) {
  bool submit_changes = false;

  ImGui::PushID(path.to_string().c_str());
  json& action = editing_copy_[path];

  ImGui::Separator();
  ImGui::BeginGroup();
  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  ImGui::PushFont(font_awesome);
  if (ImGui::SmallButton("\uf7a4")) {
  }
  ImGui::PopFont();
  // if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
  //   ImGui::SetDragDropPayload("action", &index, sizeof(index));
  //   ImGui::BeginGroup();
  //   const std::string& action_name = action["function"];
  //   ImGui::Text("%s", action_name.c_str());
  //   ImGui::EndGroup();
  //   ImGui::EndDragDropSource();
  // }

  const std::string& type = action["type"];
  if (type == "function_call") {
    submit_changes = DrawFunctionCall(path);
  } else if (type == "if") {
    submit_changes = DrawIfStatement(path);
  }

  ImGui::EndGroup();

  ImGui::Separator();

  ImGui::PopID();

  return submit_changes;
}

bool ScriptLibraryEditor::DrawFunctionCall(json::json_pointer path) {
  bool submit_changes = false;

  json& function_call = editing_copy_[path];
  const std::string& function_identifer = function_call["function"];
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
      std::string input_value = function_call["inputs"][std::to_string(input.index())];
      if (ImGui::InputText("###Value", &input_value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        function_call["inputs"][std::to_string(input.index())] = input_value;
        submit_changes = true;
      }
      ImGui::PopItemWidth();

      if (ImGui::BeginDragDropTarget()) {
        auto payload = ImGui::AcceptDragDropPayload("value");

        if (payload != nullptr) {
          const char* output_reference = reinterpret_cast<const char*>(payload->Data);
          function_call["inputs"][std::to_string(input.index())] = output_reference;
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
        auto parent_pointer = path.parent_pointer();
        auto& parent = editing_copy_[parent_pointer];
        size_t array_index = std::stoull(path.to_string().substr(parent_pointer.to_string().size() + 1));
        const std::string output_reference = fmt::format("${}:{}", array_index, output->identifier);
        ImGui::SetDragDropPayload("value", output_reference.data(), output_reference.size() + 1);
        ImGui::SmallButton(button_label.c_str());
        ImGui::EndDragDropSource();
      }

      ImGui::PopID();
      ImGui::PopID();
    }
  }

  return submit_changes;
}

bool ScriptLibraryEditor::DrawIfStatement(json::json_pointer path) {
  bool submit_changes = false;

  ImGui::Text("if ");
  ImGui::SameLine();
  ImGui::PushItemWidth(100);

  json& if_statement = editing_copy_[path];
  std::string condition = if_statement["condition"];
  if (ImGui::InputText("###Condition", &condition, ImGuiInputTextFlags_EnterReturnsTrue)) {
    if_statement["condition"] = condition;
    submit_changes = true;
  }
  ImGui::PopItemWidth();

  if (ImGui::BeginDragDropTarget()) {
    auto payload = ImGui::AcceptDragDropPayload("value");

    if (payload != nullptr) {
      const char* output_reference = reinterpret_cast<const char*>(payload->Data);
      if_statement["condition"] = condition;
      submit_changes = true;
    }

    ImGui::EndDragDropTarget();
  }

  ImGui::TreePush("Test");
  if (DrawActions(path / "actions")) {
    submit_changes = true;
  }
  ImGui::TreePop();

  return submit_changes;
}

void ScriptLibraryEditor::DrawSpace(json::json_pointer path) {
  ImGui::PushItemWidth(-1);
  std::string function_identifier;
  const std::string label = fmt::format("###Space{}", path.to_string());
  if (ImGui::InputText(label.c_str(), &function_identifier, ImGuiInputTextFlags_EnterReturnsTrue)) {
    DoOnceAfterUpdate([this, path, function_identifier]() {
      json action;
      if (function_identifier == "if") {
        action = {{"type", "if"}, { "condition", "" }, {"actions", json::array()}};
      } else {
        auto function = global_script_context.GetFunction(function_identifier);
        if (function == nullptr) {
          return;
        }

        action = {{"type", "function_call"}, { "function", function_identifier }, {"inputs", json::object()}};
        for (size_t i = 0; i < function->inputs.size(); ++i) {
          action["inputs"][std::to_string(i)] = "";
        }
      }
      json serialized_chunk = GetCurrentJsonFileState();
      auto parent_pointer = path.parent_pointer();
      auto& parent = serialized_chunk[parent_pointer];
      size_t array_index = std::stoull(path.to_string().substr(parent_pointer.to_string().size() + 1));
      parent.insert(parent.begin() + array_index, action);
      SubmitJsonFile(serialized_chunk, "json");
    });
  }
  ImGui::PopItemWidth();

  // if (ImGui::BeginDragDropTarget()) {
  //   auto payload = ImGui::AcceptDragDropPayload("action");

  //   if (payload != nullptr) {
  //     SDL_assert(payload->DataSize == sizeof(size_t));
  //     const size_t source_index = *reinterpret_cast<size_t*>(payload->Data);
  //     json serialized_chunk = GetCurrentJsonFileState();
  //     auto& actions = serialized_chunk["actions"];
  //     json action = actions[source_index];
  //     actions.erase(actions.begin() + source_index);
  //     actions.insert(actions.begin() + target_index - (target_index > source_index ? 1 : 0), std::move(action));
  //     SubmitJsonFile(serialized_chunk);
  //   }

  //   ImGui::EndDragDropTarget();
  // }
}

}  // namespace editor
}  // namespace ovis
