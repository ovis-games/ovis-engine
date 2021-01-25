#pragma once

#include <ovis/core/json.hpp>

namespace ImGui
{

bool InputJson(const char* label, ovis::json* value, const ovis::json& schema);

} // namespace ImGui