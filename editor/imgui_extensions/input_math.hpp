#pragma once

#include <ovis/math/basic_types.hpp>
#include <ovis/core/json.hpp>

namespace ImGui
{

bool InputVector2(const char* label, ovis::json* value, const ovis::json& schema, int flags = 0);
bool InputVector3(const char* label, ovis::json* value, const ovis::json& schema, int flags = 0);
bool InputVector4(const char* label, ovis::json* value, const ovis::json& schema, int flags = 0);
bool InputColor(const char* label, ovis::json* value, const ovis::json& schema, int flags = 0);

} // namespace ImGui