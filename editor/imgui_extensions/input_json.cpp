#include "input_json.hpp"

#include "input_asset.hpp"
#include <string>

#include <SDL2/SDL_assert.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <ovis/core/log.hpp>
#include <ovis/core/serialize.hpp>
#include <ovis/math/json_serialization.hpp>

namespace ImGui {

namespace {

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
      ovis::LogE("Invalid schema reference. Pointer '{}' does not exist in {}", schema_reference, schema_file_reference);
      return false;
    }
    ovis::json referenced_schema = referenced_schema_file->at(schema_reference_pointer);
    // TODO: patch referenced_schema with values overriden in 'schema' variable

    json_changed = InputJson(label, value, referenced_schema, flags, depth);
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
        const auto& properties = schema["properties"];
        if (properties.is_object()) {
          for (auto property = properties.begin(), end = properties.end(); property != end; ++property) {
            try {
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
            } catch (...) {
              // value->at(property.key()) may throw if the value does not exists
              // Should we ignore it?
              ovis::LogV("Missing property '{}' in {}", property.key(), value->dump());
            }
          }
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
    } else if (type == "vector2") {
      glm::vec2 vector = *value;
      if (ImGui::DragFloat2(label, glm::value_ptr(vector), 1.0f, 0.0f, 0.0f, "%.2f")) {
        *value = vector;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "vector3") {
      glm::vec3 vector = *value;
      if (ImGui::DragFloat3(label, glm::value_ptr(vector), 1.0f, 0.0f, 0.0f, "%.2f")) {
        *value = vector;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "vector4") {
      glm::vec4 vector = *value;
      if (ImGui::DragFloat4(label, glm::value_ptr(vector), 1.0f, 0.0f, 0.0f, "%.2f")) {
        *value = vector;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "color") {
      glm::vec4 color = *value;
      if (ImGui::ColorEdit4(label, glm::value_ptr(color))) {
        *value = color;
        json_changed = true;
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

}  // namespace ImGui
