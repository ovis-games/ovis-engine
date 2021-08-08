#include "script_library_editor.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scripting.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {
namespace editor {

namespace {

template <typename T>
void ParseFunctionText(const ScriptFunction* function, std::string_view function_text, T&& callback) {
  assert(function != nullptr);

  std::string_view remaining = function_text;
  bool inside_brace = false;
  do {
    if (!inside_brace) {
      const auto brace_pos = remaining.find('(');
      const auto text_before_brace = remaining.substr(0, brace_pos);
      callback(text_before_brace);
      if (brace_pos != std::string_view::npos) {
        inside_brace = true;
        remaining = remaining.substr(brace_pos + 1);
      } else {
        remaining = "";
      }
    } else {
      const auto brace_pos = remaining.find(')');
      const auto input_identifier = remaining.substr(0, brace_pos);

      std::optional<ScriptTypeId> type;
      for (const auto& input : function->inputs) {
        if (input.identifier == input_identifier) {
          type = input.type;
          break;
        }
      }
      if (!type) {
        LogE("Unknown input {} for function text {}", input_identifier, function_text);
      } else {
        callback(ScriptValueDefinition{
            .type = *type,
            .identifier = std::string(input_identifier),
        });
      }

      if (brace_pos != std::string_view::npos) {
        inside_brace = false;
        remaining = remaining.substr(brace_pos + 1);
      } else {
        LogE("Invalid function text: {}", function_text);
        remaining = "";
      }
    }
  } while (remaining.size() > 0);
}

}

ScriptLibraryEditor::ScriptLibraryEditor(const std::string& asset_id) : AssetEditor(asset_id) {
  SetupJsonFile({{"actions", json::array()}});

  auto core_defition = GetEngineAssetLibrary()->LoadAssetTextFile("core_definitions", "json");
  if (core_defition.has_value()) {
    docs_ = json::parse(*core_defition);
  }
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
    const int action_id = action["id"];
    if (error_.has_value() && error_->action_id == action_id) {
      EndNode(true);
    } else {
      EndNode();
    }
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
    if (error_.has_value() && error_->action_id == action_id) {
      ImGui::SameLine();
      ImGui::TextColored(ImColor(204, 0, 0, 255), "%s", error_->message.c_str());
    }
  }

  ImGui::PopID();

  return submit_changes;
}

bool ScriptLibraryEditor::DrawFunctionCall(const json::json_pointer& path) {
  bool submit_changes = false;

  json& function_call = editing_copy_[path];
  const std::string& function_identifer = function_call["function"];
  const ScriptFunction* function = global_script_context()->GetFunction(function_identifer);
  if (function == nullptr) {
    ImGui::Text("Function definition not found for: %s", function_identifer.c_str());
  } else if (auto doc = docs_.find(function_identifer); doc != docs_.end()) {
    const std::string& text = doc.value()["text"];

    ParseFunctionText(function, text, [&, this](std::variant<std::string_view, ScriptValueDefinition> text_or_input) {
      if (std::holds_alternative<std::string_view>(text_or_input)) {
        const std::string_view text = std::get<0>(text_or_input);
        ImGui::TextUnformatted(text.begin(), text.end());
      } else if (std::holds_alternative<ScriptValueDefinition>(text_or_input)) {
        const auto input = std::get<1>(text_or_input);
        if (DrawInput(path / "inputs" / input.identifier, input.type)) {
          submit_changes = true;
        }
      }
      ImGui::SameLine();
    });

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
  if (DrawInput(path / "condition", global_script_context()->GetTypeId<bool>())) {
    submit_changes = true;
  }

  return submit_changes;
}

bool ScriptLibraryEditor::DrawNewAction(const json::json_pointer& path) {
  bool submit_changes = false;

  std::string text;
  json& action = editing_copy_[path];
  const std::string label = fmt::format("###{}", path.to_string());

  // TODO: See implementation at https://gist.github.com/harold-b/7dcc02557c2b15d76c61fde1186e31d0
  if (ImGui::InputText(label.c_str(), &text)) {
  }

  const auto was_active_last_frame_id = ImGui::GetID("was_active_last_frame");
  const auto storage = ImGui::GetStateStorage();
  const bool was_active_last_frame = storage->GetBool(was_active_last_frame_id, true);

  bool is_active;
  if (start_editing_) {
    ImGui::SetKeyboardFocusHere();
    start_editing_ = false;
    is_active = true;
  } else {
    is_active = ImGui::IsItemActive() || ImGui::IsItemDeactivated();
  }

  if (is_active || was_active_last_frame) {
    ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
    ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x, 0});
    if (ImGui::Begin("##autocomplete", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_Tooltip)) {
      ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
      is_active = is_active || ImGui::IsWindowFocused();

      const bool alt_down = ImGui::GetIO().KeyAlt;

      for (const auto& function_documentation : docs_.items()) {
        const auto& function_identifier = function_documentation.key();
        const std::string& function_text = function_documentation.value()["text"];

        if (!std::strstr(function_identifier.c_str(), text.c_str()) && !std::strstr(function_text.c_str(), text.c_str())) {
          continue;
        }

        auto function = global_script_context()->GetFunction(function_identifier);
        if (function == nullptr) {
          continue;
        }

        std::string final_text;
        ParseFunctionText(function, function_text, [&, this](std::variant<std::string_view, ScriptValueDefinition> text_or_input) {
          if (std::holds_alternative<std::string_view>(text_or_input)) {
            const std::string_view text = std::get<0>(text_or_input);
            final_text += text;
          } else if (std::holds_alternative<ScriptValueDefinition>(text_or_input)) {
            const auto input = std::get<1>(text_or_input);
            if (alt_down) {
              final_text += fmt::format("{}[{}]", input.identifier, global_script_context()->GetType(input.type)->name);
            } else {
              final_text += input.identifier;
            }
          }
        });
        final_text += fmt::format("##{}", function_identifier);

        if (ImGui::Selectable(final_text.c_str())) {
          action["type"] = "function_call";
          action["function"] = function_identifier;
          action["inputs"] = json::object();

          for (const auto& input : function->inputs) {
            action["inputs"][input.identifier] = json{};
          }
          action["outputs"] = json::object();
          for (const auto& output : function->outputs) {
            action["outputs"][output.identifier] = json{};
          }

          submit_changes = true;
        }
      }
    }
    ImGui::End();
  }

  storage->SetBool(was_active_last_frame_id, is_active);

  if (!is_active && !was_active_last_frame) {
    DoOnceAfterUpdate([this, path]() {
      RemoveAction(path);
      SubmitJsonFile(editing_copy_);
    });
  }

  return submit_changes;
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

void ScriptLibraryEditor::EndNode(bool draw_error_border) {
  ImGui::EndGroup();
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
  ImGui::NewLine();
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
  ImGui::EndGroup();

  ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
  ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(80, 80, 80, 255),
                                            5.0f);
  if (draw_error_border) {
    ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(204, 0, 0, 255),
                                        5.0f);
  }

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
    const auto& outputs = action["outputs"];
    if (outputs.contains(ref->output) && outputs[ref->output].is_string()) {
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
