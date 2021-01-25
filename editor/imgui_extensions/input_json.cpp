#include "input_json.hpp"
#include "input_asset.hpp"

#include <string>

#include <SDL2/SDL_assert.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <ovis/core/log.hpp>
#include <ovis/core/serialize.hpp>
#include <ovis/math/serialize.hpp>
#include <ovis/math/vector.hpp>

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

}  // namespace

bool InputJson(const char* label, ovis::json* value, const ovis::json& schema) {
  SDL_assert(value != nullptr);
  bool json_changed = false;

  if (schema.contains("type")) {
    const std::string type = schema["type"];
    if (type == "object") {
      if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        DisplayTooltip(schema);

        const auto& properties = schema["properties"];
        if (properties.is_object()) {
          for (auto property = properties.begin(), end = properties.end(); property != end; ++property) {
            try {
              if (InputJson(property.key().c_str(), &value->at(property.key()), property.value())) {
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
    } else if (type == "number") {
      float object = *value;
      if (ImGui::InputFloat(label, &object, 0.0f, 0.0f, "%0.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        *value = object;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "vector2") {
      glm::vec2 vector = *value;
      if (ImGui::InputFloat2(label, glm::value_ptr(vector), "%0.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        *value = vector;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "vector3") {
      glm::vec3 vector = *value;
      if (ImGui::InputFloat3(label, glm::value_ptr(vector), "%0.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        *value = vector;
        json_changed = true;
      }
      DisplayTooltip(schema);
    } else if (type == "vector4") {
      glm::vec4 vector = *value;
      if (ImGui::InputFloat4(label, glm::value_ptr(vector), "%0.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
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
    } else if (type.size() > 7 && strncmp(type.c_str(), "asset<", 5) == 0 && type.back() == '>') { // 7 = strlen("asset<>"")
      const std::string asset_type = type.substr(6, type.size()-7);
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

}  // namespace ImGui
