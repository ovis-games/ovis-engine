#include "script_library_editor.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <ovis/core/scene.hpp>
#include <ovis/core/scripting.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {
namespace editor {

ScriptLibraryEditor::ScriptLibraryEditor(const std::string& asset_id) : AssetEditor(asset_id) {
  SetupJsonFile({{"actions", json::array()}});

  docs_["add"] = {
      {"text", "{x} + {y}"},
  };
  docs_["subtract"] = {
      {"text", "{x} - {y}"},
  };
  docs_["multiply"] = {
      {"text", "{x} * {y}"},
  };
  docs_["divide"] = {
      {"text", "{x} / {y}"},
  };
  docs_["negate"] = {
      {"text", "- {x}"},
  };
  docs_["is_greater"] = {
      {"text", "Is {x} greater than {y}"},
  };
  docs_["print"] = {
      {"text", "Print {value}"},
  };
  docs_["print_bool"] = {
      {"text", "Print {value}"},
  };
}

void ScriptLibraryEditor::DrawContent() {
  if (ImGui::Button("Run")) {
    chunk_.emplace(editing_copy_);
    chunk_->Print();
    chunk_->Execute({});
  }
  ImGui::Separator();

  highlighted_reference_ = reference_to_highlight_;
  reference_to_highlight_ = "";
  if (ImGui::BeginChild("script", ImVec2(-1, -1), 0, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
    if (DrawEntrypoint()) {
      SubmitJsonFile(editing_copy_, "json");
    }
    if (DrawActions(json::json_pointer("/actions"))) {
      SubmitJsonFile(editing_copy_, "json");
    }
  }
  ImGui::EndChild();
}

bool ScriptLibraryEditor::DrawEntrypoint() {
  bool submit_changes = false;

  BeginNode();
  ImGui::Text("Inputs: ");
  for (auto& input : editing_copy_["inputs"].items()) {
    const auto path = json::json_pointer("/inputs") / input.key();
    std::string name = editing_copy_[path]["name"];
    const std::string reference = fmt::format("$inputs:{}", input.key());

    ImGui::SameLine();
    if (DrawRenameableValueSource(path.to_string().c_str(), &name, {}, reference)) {
      editing_copy_[path]["name"] = name;
      submit_changes = true;
    }
  }

  ImGui::Text("Outputs");
  for (auto& output : editing_copy_["outputs"].items()) {
  }

  EndNode();
  return submit_changes;
}

bool ScriptLibraryEditor::DrawActions(const json::json_pointer& path) {
  auto& actions = editing_copy_[path];

  bool submit_changes = false;
  for (auto& action : IndexRange(actions)) {
    DrawSpace(path / action.index());
    if (DrawAction(path / action.index())) {
      submit_changes = true;
    }
    if (action->contains("actions")) {
      ImGui::PushID(path.to_string().c_str());
      ImGui::TreePush("actions");
      if (DrawActions(path / action.index() / "actions")) {
        submit_changes = true;
      }
      ImGui::TreePop();
      ImGui::PopID();
    }
  }
  DrawSpace(path / actions.size());
  return submit_changes;
}

bool ScriptLibraryEditor::DrawAction(const json::json_pointer& path, bool dragging_preview) {
  bool submit_changes = false;

  ImGui::PushID(path.to_string().c_str());
  json& action = editing_copy_[path];

  bool is_dragging = !dragging_preview && dragged_action_path_ == path;

  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");
  if (!is_dragging) {
    BeginNode();
    ImGui::PushFont(font_awesome);
    ImGui::SmallButton("\uf7a4###Placeholder");
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    }
    ImGui::PopFont();
    ImGui::SameLine();
  } else {
    ImGui::InvisibleButton("###Placeholder", ImVec2(1, 1));
  }
  if (!dragging_preview) {
    if (ImGui::BeginDragDropSource()) {
      dragged_action_path_ = path;
      const size_t id = action["id"];
      ImGui::SetDragDropPayload("action", &id, sizeof(id));
      DrawAction(path, true);
      ImGui::EndDragDropSource();
    } else if (dragged_action_path_ == path) {
      dragged_action_path_ = json::json_pointer{};
    }
  }
  if (!is_dragging) {
    const std::string& type = action["type"].is_string() ? action["type"] : "";
    if (type == "function_call") {
      submit_changes = DrawFunctionCall(path);
    } else if (type == "if") {
      submit_changes = DrawIfStatement(path);
    } else {
      submit_changes = DrawNewAction(path);
    }
    EndNode();
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    ImGui::PushFont(font_awesome);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));
    if (ImGui::SmallButton("\uf1f8")) {
      DoOnceAfterUpdate([this, path]() {
        RemoveAction(path);
        SubmitJsonFile(editing_copy_);
      });
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    ImGui::PopStyleColor(2);
    ImGui::PopFont();
  }

  ImGui::PopID();

  return submit_changes;
}

bool ScriptLibraryEditor::DrawFunctionCall(const json::json_pointer& path) {
  bool submit_changes = false;

  json& function_call = editing_copy_[path];
  const std::string& function_identifer = function_call["function"];
  const ScriptFunction* function = global_script_context.GetFunction(function_identifer);
  if (function == nullptr) {
    ImGui::Text("Function definition not found for: %s", function_identifer.c_str());
  } else if (auto doc = docs_.find(function_identifer); doc != docs_.end()) {
    const std::string text = doc->second["text"];
    std::istringstream iss(text);
    while (iss) {
      std::string word;
      iss >> word;

      if (word.front() == '{' && word.back() == '}') {
        const std::string input_identifier = word.substr(1, word.length() - 2);
        const auto input_path = path / "inputs" / input_identifier;
        if (DrawInput(input_path, {})) {
          submit_changes = true;
        }
      } else {
        ImGui::Text("%s", word.c_str());
      }
      ImGui::SameLine();
    }

    if (function->outputs.size() == 0) {
      ImGui::Text(".");
    } else {
      ImGui::Text(":");
      for (const auto& output : IndexRange(function->outputs)) {
        ImGui::SameLine();
        const auto output_path = path / "outputs" / output->identifier;
        const std::string reference =
            fmt::format("${}:{}", static_cast<size_t>(function_call["id"]), output->identifier);
        std::string output_name = GetReferenceName(reference);
        if (DrawRenameableValueSource(output_path.to_string().c_str(), &output_name, ScriptTypeId{}, reference)) {
          editing_copy_[output_path] = output_name;
          submit_changes = true;
        }
      }
    }
  } else {
    // ImGui::Text("%s", function_identifer.c_str());
    // ImGui::Text("Inputs: ");

    // for (const auto& input : IndexRange(function->inputs)) {
    //   ImGui::PushID("Inputs");
    //   ImGui::PushID(input.index());

    //   ImGui::Text("%s : %s = ", input.value().identifier.c_str(), "Number");
    //   ImGui::SameLine();

    //   ImGui::PushItemWidth(100);
    //   std::string input_value = function_call["inputs"][std::to_string(input.index())];
    //   if (ImGui::InputText("###Value", &input_value, ImGuiInputTextFlags_EnterReturnsTrue)) {
    //     function_call["inputs"][std::to_string(input.index())] = input_value;
    //     submit_changes = true;
    //   }
    //   ImGui::PopItemWidth();

    //   ImGui::PopID();
    //   ImGui::PopID();
    // }
    // ImGui::Text("Outputs: ");
    // for (const auto& output : IndexRange(function->outputs)) {
    //   ImGui::PushID("Output");
    //   ImGui::PushID(output.index());

    //   const std::string button_label = fmt::format("{} : {}", output->identifier, "Number");
    //   ImGui::SmallButton(button_label.c_str());

    //   if (ImGui::BeginDragDropSource()) {
    //     auto parent_pointer = path.parent_pointer();
    //     auto& parent = editing_copy_[parent_pointer];
    //     size_t array_index = std::stoull(path.to_string().substr(parent_pointer.to_string().size() + 1));
    //     const std::string output_reference = fmt::format("${}:{}", array_index, output->identifier);
    //     ImGui::SetDragDropPayload("value", output_reference.data(), output_reference.size() + 1);
    //     ImGui::SmallButton(button_label.c_str());
    //     ImGui::EndDragDropSource();
    //   }

    //   ImGui::PopID();
    //   ImGui::PopID();
    // }
  }

  return submit_changes;
}

bool ScriptLibraryEditor::DrawIfStatement(const json::json_pointer& path) {
  bool submit_changes = false;

  ImGui::Text("if ");
  ImGui::SameLine();
  if (DrawInput(path / "condition", {})) {
    submit_changes = true;
  }

  return submit_changes;
}

bool ScriptLibraryEditor::DrawNewAction(const json::json_pointer& path) {
  std::string text;
  json& action = editing_copy_[path];
  const std::string label = fmt::format("###{}", path.to_string());
  if (ImGui::InputText(label.c_str(), &text, ImGuiInputTextFlags_EnterReturnsTrue)) {
    if (text == "if") {
      action["type"] = "if";
      action["condition"] = "";
      action["actions"] = json::array();
    } else {
      auto function = global_script_context.GetFunction(text);
      if (function == nullptr) {
        DoOnceAfterUpdate([this, path]() {
          RemoveAction(path);
          SubmitJsonFile(editing_copy_);
        });
        return false;
      }

      action["type"] = "function_call";
      action["function"] = text;
      action["inputs"] = json::object();
      for (const auto& input : function->inputs) {
        action["inputs"][input.identifier] = json{};
      }
      action["outputs"] = json::object();
      for (const auto& output : function->outputs) {
        action["outputs"][output.identifier] = json{};
      }
    }

    return true;
  }
  if (start_editing_) {
    ImGui::SetKeyboardFocusHere();
    start_editing_ = false;
  } else if (!ImGui::IsItemActive()) {
    DoOnceAfterUpdate([this, path]() {
      RemoveAction(path);
      SubmitJsonFile(editing_copy_);
    });
  }

  return false;
}

void ScriptLibraryEditor::DrawSpace(const json::json_pointer& path) {
  if (dragged_action_path_ == path) {
    return;
  }

  std::string function_identifier;
  const std::string label = fmt::format("+###Space{}", path.to_string());
  ImGui::SetNextItemWidth(-1);
  if (ImGui::InvisibleButton(label.c_str(), ImVec2(-1.0f, 15.0f))) {
    DoOnceAfterUpdate([this, path, function_identifier]() {
      json action;
      action["id"] = GenerateActionId();
      auto parent_pointer = path.parent_pointer();
      auto& parent = editing_copy_[parent_pointer];
      size_t array_index = std::stoull(path.to_string().substr(parent_pointer.to_string().size() + 1));
      parent.insert(parent.begin() + array_index, action);
      start_editing_ = true;
    });
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  }

  if (ImGui::BeginDragDropTarget()) {
    auto payload = ImGui::AcceptDragDropPayload("action");

    if (payload != nullptr) {
      SDL_assert(payload->DataSize == sizeof(size_t));
      const size_t source_id = *reinterpret_cast<size_t*>(payload->Data);
      json::json_pointer old_action_path = GetActionWithId(source_id);
      DoOnceAfterUpdate([this, path, old_action_path]() {
        json action = editing_copy_[old_action_path];

        const size_t temporary_id_for_old_action = GenerateActionId();
        editing_copy_[old_action_path]["id"] = temporary_id_for_old_action;

        auto parent_pointer = path.parent_pointer();
        auto& parent = editing_copy_[parent_pointer];
        size_t array_index = std::stoull(path.to_string().substr(parent_pointer.to_string().size() + 1));
        parent.insert(parent.begin() + array_index, action);

        // The path of the old action could have changed, so get path again
        RemoveAction(GetActionWithId(temporary_id_for_old_action));

        SubmitJsonFile(editing_copy_);
      });
    }

    ImGui::EndDragDropTarget();
  }
}

bool ScriptLibraryEditor::DrawInput(const json::json_pointer& path, ScriptTypeId type) {
  bool submit_changes = false;
  json& value = editing_copy_[path];
  const std::string identifier = path.to_string().substr(path.parent_pointer().to_string().size() + 1);

  std::string string_value = value.is_string() ? value : "";
  if (current_edit_path_ == path) {
    ImGui::PushItemWidth(100);
    const std::string text_label = fmt::format("###{}", path.to_string());
    if (ImGui::InputText(text_label.c_str(), &string_value, ImGuiInputTextFlags_EnterReturnsTrue)) {
      current_edit_path_ = json::json_pointer();
      value = string_value;
      submit_changes = true;
    }
    if (start_editing_) {
      ImGui::SetKeyboardFocusHere(-1);
      start_editing_ = false;
    } else if (!ImGui::IsItemActive()) {
      current_edit_path_ = json::json_pointer();
      value = string_value;
      submit_changes = true;
    }
    ImGui::PopItemWidth();
  } else {
    bool highlight = false;
    bool enable_drag = true;
    std::string display_value;
    if (value.is_string()) {
      if (string_value.length() > 0 && string_value[0] == '$') {
        highlight = string_value == highlighted_reference_;
        display_value = GetReferenceName(string_value);
      } else {
        display_value = string_value;
      }
    } else {
      string_value = fmt::format("[{}]", identifier);
      enable_drag = false;
    }
    const std::string button_label = fmt::format("{}###{}", display_value, identifier);
    if (highlight) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    }
    if (ImGui::SmallButton(button_label.c_str())) {
      current_edit_path_ = path;
      start_editing_ = true;
    }
    if (ImGui::IsItemHovered()) {
      reference_to_highlight_ = string_value;
    }
    if (highlight) {
      ImGui::PopStyleColor();
    }
    if (ImGui::BeginDragDropSource()) {
      ImGui::SetDragDropPayload("value", string_value.data(), string_value.size());
      ImGui::SmallButton(display_value.c_str());
      ImGui::EndDragDropSource();
    }
  }
  if (ImGui::BeginDragDropTarget()) {
    auto payload = ImGui::AcceptDragDropPayload("value");

    if (payload != nullptr) {
      const char* output_reference = reinterpret_cast<const char*>(payload->Data);
      value = std::string(output_reference, output_reference + payload->DataSize);
      submit_changes = true;
    }

    ImGui::EndDragDropTarget();
  }

  return submit_changes;
}

void ScriptLibraryEditor::BeginNode() {
  ImGui::GetWindowDrawList()->ChannelsSplit(2);
  ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
  ImGui::BeginGroup();
  ImGui::Indent(10.0f);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
  ImGui::BeginGroup();
}

void ScriptLibraryEditor::EndNode() {
  ImGui::EndGroup();
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
  ImGui::NewLine();
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
  ImGui::EndGroup();

  ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
  ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(80, 80, 80, 255),
                                            5.0f);

  ImGui::GetWindowDrawList()->ChannelsMerge();
}

bool ScriptLibraryEditor::DrawRenameableValueSource(const char* id, std::string* name, ScriptTypeId type,
                                                    std::string_view reference) {
  bool name_changed = false;

  ImGui::PushID(id);

  const auto state_id = ImGui::GetID("State");
  const auto storage = ImGui::GetStateStorage();
  const int state = storage->GetInt(state_id);
  const bool is_editing = state != 0;
  const bool started_editing = state == 1;

  if (is_editing) {
    ImGui::PushItemWidth(100);
    if (ImGui::InputText("###OutputName", name, ImGuiInputTextFlags_EnterReturnsTrue)) {
      name_changed = true;
    }
    if (started_editing) {
      ImGui::SetKeyboardFocusHere(-1);
      storage->SetInt(state_id, 2);
    } else if (!ImGui::IsItemActive()) {
      storage->SetInt(state_id, 0);
    }
    ImGui::PopItemWidth();
  } else {
    if (highlighted_reference_ == reference) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    }
    if (ImGui::SmallButton(name->c_str())) {
      storage->SetInt(state_id, 1);
    }
    if (highlighted_reference_ == reference) {
      ImGui::PopStyleColor();
    }
    if (ImGui::IsItemHovered()) {
      reference_to_highlight_ = reference;
    }
  }

  if (ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload("value", reference.data(), reference.size());
    ImGui::SmallButton(name->c_str());
    ImGui::EndDragDropSource();
  }

  ImGui::PopID();

  return name_changed;
}

void ScriptLibraryEditor::RemoveAction(const json::json_pointer& path) {
  const auto parent_path = path.parent_pointer();
  const size_t array_index = std::stoull(path.to_string().substr(parent_path.to_string().size() + 1));
  json& parent = editing_copy_[parent_path];
  parent.erase(parent.begin() + array_index);
}

void ScriptLibraryEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  if (file_type == "json") {
    editing_copy_ = data;
  }
}

json::json_pointer ScriptLibraryEditor::GetActionWithId(size_t id, json::json_pointer base_path) {
  for (const auto& action : IndexRange(editing_copy_[base_path])) {
    if (action.value()["id"] == id) {
      return base_path / action.index();
    } else if (action->contains("actions")) {
      const auto path = GetActionWithId(id, base_path / action.index() / "actions");
      if (!path.empty()) {
        return path;
      }
    }
  }

  return json::json_pointer{};
}

std::string ScriptLibraryEditor::GetReferenceName(std::string_view reference) {
  const auto ref = ScriptReference::Parse(reference);
  if (!ref.has_value()) {
    return "";
  }

  if (ref->node_id == "inputs") {
    return editing_copy_["inputs"][ref->output]["name"];
  } else if (ref->node_id == "outputs") {
  } else {
    const auto action_path = GetActionWithId(std::stoull(ref->node_id));
    if (action_path.empty()) {
      return "";
    }

    const auto& action = editing_copy_[action_path];
    if (action["outputs"][ref->output].is_string()) {
      return action["outputs"][ref->output];
    }
  }

  return ref->output;
}

size_t ScriptLibraryEditor::GenerateActionId() {
  size_t id;
  do {
    id = rand();
  } while (!GetActionWithId(id).empty());
  return id;
}

}  // namespace editor
}  // namespace ovis
