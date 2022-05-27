#pragma once

#include "ovis/vm/script_function_parser.hpp"
#include "ovis/vm/script_type_parser.hpp"

namespace ovis {

struct ParseScriptResult {
  std::vector<ParseScriptFunctionResult> functions;
  std::vector<ParseScriptTypeResult> types;
};

Result<ParseScriptResult, ParseScriptErrors> ParseScript(VirtualMachine* virtual_machine, const json& script);

struct ScriptParser {
};

}  // namespace ovis
