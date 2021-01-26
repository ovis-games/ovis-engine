#pragma once

#include <ovis/core/json.hpp>

enum ImGuiInputJsonFlags {

ImGuiInputJsonFlags_IgnoreEnclosingObject = 1

};

namespace ImGui
{

bool InputJson(const char* label, ovis::json* value, const ovis::json& schema, int flags = 0);

} // namespace ImGui