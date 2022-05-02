#pragma once

#include <string>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/vm/function.hpp>
#include <ovis/vm/parse_script_error.hpp>
#include <ovis/vm/type.hpp>
#include <ovis/vm/value.hpp>
#include <ovis/vm/virtual_machine.hpp>

namespace ovis {

struct ParseScriptTypeResult {
  TypeDescription type_description;
  // FunctionDescription construct_function;
  // FunctionDescription copy_function;
  // FunctionDescription destruct_function;
};

Result<ParseScriptTypeResult, ParseScriptErrors> ParseScriptType(VirtualMachine* virtual_machine, const json& type_definition);

}
