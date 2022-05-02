#pragma once

#include <string>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/vm/function.hpp>
#include <ovis/vm/type.hpp>
#include <ovis/vm/value.hpp>
#include <ovis/vm/parse_script_error.hpp>

namespace ovis {

struct ParseScriptFunctionResult {
  FunctionDescription function_description;
  // ScriptFunction::DebugInfo debug_info;
};

Result<ParseScriptFunctionResult, ParseScriptErrors> ParseScriptFunction(VirtualMachine* virtual_machine, const json& function_definition);

}  // namespace ovis
