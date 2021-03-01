#include "input_json.hpp"

#include "input_asset.hpp"
#include <string>

#include <SDL2/SDL_assert.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <ovis/core/log.hpp>
#include <ovis/core/serialize.hpp>
#include <ovis/math/basic_types.hpp>

namespace ImGui {

namespace {

std::unordered_map<std::string, InputJsonFunction> custom_functions;

void DisplayTooltip(const ovis::json& schema) {
  if (ImGui::IsItemHovered()) {
    if (schema.contains("description")) {
      const std::string& description = schema["description"];
      ImGui::SetTooltip("%s", description.c_str());
    }
  }
}

bool InputJson(const char* label, ovis::json* value, const ovis::json& schema, int flags, int depth) {
  SDL_assert(value != nullptr);
  bool json_changed = false;

  if (schema.contains("$ref")) {
    const std::string reference = schema["$ref"];
    if (custom_functions.count(reference) == 1) {
      json_changed = custom_functions[reference](label, value, schema, flags);
    } else {
      const unsigned hashtag_position = reference.find('#');
      if (hashtag_position == std::string::npos) {
        ovis::LogE("Invalid JSON reference: {}", reference);
        return false;
      }
      const std::string schema_file_reference = reference.substr(0, hashtag_position);
      const std::string schema_reference = reference.substr(hashtag_position + 1);

      std::shared_ptr<ovis::json> referenced_schema_file = ovis::LoadJsonSchema(schema_file_reference);
      if (!referenced_schema_file) {
        ovis::LogE("Failed to load schema: {}", schema_file_reference);
        return false;
      }

      ovis::json::json_pointer schema_reference_pointer(schema_reference);
      if (!referenced_schema_file->contains(schema_reference_pointer)) {
        ovis::LogE("Invalid schema reference. Pointer '{}' does not exist in {}", schema_reference,
                   schema_file_reference);
        return false;
      }
      ovis::json referenced_schema = referenced_schema_file->at(schema_reference_pointer);
      // TODO: patch referenced_schema with values overriden in 'schema' variable

      json_changed = InputJson(label, value, referenced_schema, flags, depth);
    }
  } else if (schema.contains("type")) {
    const std::string type = schema["type"];
    if (type == "object") {
      bool display_properties = true;
      bool pushed = false;

      if ((flags & ImGuiInputJsonFlags_IgnoreEnclosingObject) == 0) {
        if (depth > 0) {
          ImGui::TreePush(label);
          pushed = true;
        }

        display_properties = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
        DisplayTooltip(schema);
      }

      if (display_properties) {
        ImGui::BeginGroup();

        const ovis::json& properties = schema.contains("properties") ? schema["properties"] : ovis::json::object();
        if (properties.is_object()) {
          for (auto property = properties.begin(), end = properties.end(); property != end; ++property) {
            if (value->contains(property.key())) {
              const int new_depth = flags & ImGuiInputJsonFlags_IgnoreEnclosingObject ? depth : depth + 1;
              std::string label;
              if (property.value().contains("title")) {
                label = property.value().at("title");
              } else {
                label = property.key();
              }

              if (InputJson(label.c_str(), &value->at(property.key()), property.value(),
                            flags & (~ImGuiInputJsonFlags_IgnoreEnclosingObject), new_depth)) {
                json_changed = true;
              }
            }
          }
        }
        
        ImGui::EndGroup();
      } else {
        DisplayTooltip(schema);
      }

      if (pushed) {
        ImGui::TreePop();
      }
    } else if (type == "array") {
      bool display_items = true;
      bool pushed = false;

      if ((flags & ImGuiInputJsonFlags_IgnoreEnclosingObject) == 0) {
        if (depth > 0) {
          ImGui::TreePush(label);
          pushed = true;
        }

        display_items = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
        DisplayTooltip(schema);
      }

      if (display_items) {
        const ovis::json& items = schema.contains("items") ? schema["items"] : ovis::json::object();
        if (value->is_array()) {
          ImGui::BeginGroup();
          const int new_depth = flags & ImGuiInputJsonFlags_IgnoreEnclosingObject ? depth : depth + 1;
          for (size_t i = 0; i < value->size(); ++i) {
            ovis::json array_item = value->at(i);
            std::string item_label = std::string(label) + "[" + std::to_string(i) + "]";
            if (InputJson(item_label.c_str(), &array_item, items, flags & (~ImGuiInputJsonFlags_IgnoreEnclosingObject),
                          new_depth)) {
              value->at(i) = array_item;
              json_changed = true;
            }
          }
          ImGui::EndGroup();
        }
      } else {
        DisplayTooltip(schema);
      }

      if (pushed) {
        ImGui::TreePop();
      }
    } else if (type == "number") {
      float number = *value;
      if (ImGui::DragFloat(label, &number, 1.0f, 0.0f, 0.0f, "%.2f")) {
        *value = number;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "boolean") {
      bool boolean = *value;
      // const std::string cb_label = std::string("###") + label;
      if (ImGui::Checkbox(label, &boolean)) {
        *value = boolean;
        json_changed = true;
      }
      // ImGui::SameLine();
      // ImGui::LabelText(label, "");
      DisplayTooltip(schema);
    } else if (type == "string") {
      std::string string = *value;

      if (schema.contains("enum")) {
        std::vector<std::string> enum_values = schema.at("enum");
        std::vector<const char*> enum_values_c_str(enum_values.size());
        int current_item = 0;
        for (int i = 0; i < enum_values.size(); ++i) {
          enum_values_c_str[i] = enum_values[i].c_str();
          if (enum_values[i] == string) {
            current_item = i;
          }
        }
        if (ImGui::Combo(label, &current_item, enum_values_c_str.data(), enum_values_c_str.size())) {
          *value = enum_values[current_item];
          json_changed = true;
        }
      } else {
        if (ImGui::InputText(label, &string)) {
          *value = string;
          json_changed = true;
        }
      }

      DisplayTooltip(schema);
    } else if (type.size() > 7 && strncmp(type.c_str(), "asset<", 5) == 0 &&
               type.back() == '>') {  // 7 = strlen("asset<>"")
      const std::string asset_type = type.substr(6, type.size() - 7);
      std::string asset_id = *value;
      if (InputAsset(label, &asset_id, asset_type)) {
        *value = asset_id;
        json_changed = true;
      }
      DisplayTooltip(schema);
    }
  }

  return json_changed;
}

}  // namespace

bool InputJson(const char* label, ovis::json* value, const ovis::json& schema, int flags) {
  return InputJson(label, value, schema, flags, 0);
}

void SetCustomJsonFunction(const std::string& schema_reference, InputJsonFunction function) {
  custom_functions[schema_reference] = function;
}

}  // namespace ImGui
