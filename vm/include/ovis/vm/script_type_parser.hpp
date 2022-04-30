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
  std::vector<Value> construct_constants;
  std::vector<Instruction> construct_instructions;
  std::vector<Value> copy_constants;
  std::vector<Instruction> copy_instructions;
  std::vector<Value> destruct_constants;
  std::vector<Instruction> destruct_instructions;
};

Result<ParseScriptTypeResult, ParseScriptErrors> ParseScriptType(VirtualMachine* virtual_machine, const json& type_definition);

}
