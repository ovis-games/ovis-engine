#include "input_math.hpp"

#include <SDL2/SDL_assert.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

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

bool InputVector2(const char* label, ovis::json* value, const ovis::json& schema, int flags) {
  SDL_assert(value != nullptr);
  bool json_changed = false;

  ovis::vector2 vector = *value;
  if (ImGui::DragFloat2(label, glm::value_ptr(vector), 1.0f, 0.0f, 0.0f, "%.2f")) {
    *value = vector;
    json_changed = true;
  }
  DisplayTooltip(schema);

  return json_changed;
}

bool InputVector3(const char* label, ovis::json* value, const ovis::json& schema, int flags) {
  SDL_assert(value != nullptr);
  bool json_changed = false;

  ovis::vector3 vector = *value;
  if (ImGui::DragFloat3(label, glm::value_ptr(vector), 1.0f, 0.0f, 0.0f, "%.2f")) {
    *value = vector;
    json_changed = true;
  }
  DisplayTooltip(schema);

  return json_changed;
}

bool InputVector4(const char* label, ovis::json* value, const ovis::json& schema, int flags) {
  SDL_assert(value != nullptr);
  bool json_changed = false;

  ovis::vector4 vector = *value;
  if (ImGui::DragFloat4(label, glm::value_ptr(vector), 1.0f, 0.0f, 0.0f, "%.2f")) {
    *value = vector;
    json_changed = true;
  }
  DisplayTooltip(schema);

  return json_changed;
}

bool InputColor(const char* label, ovis::json* value, const ovis::json& schema, int flags) {
  SDL_assert(value != nullptr);
  bool json_changed = false;

  ovis::vector4 color = *value;
  if (ImGui::ColorEdit4(label, glm::value_ptr(color))) {
    *value = color;
    json_changed = true;
  }
  DisplayTooltip(schema);

  return json_changed;
}

}  // namespace ImGui
