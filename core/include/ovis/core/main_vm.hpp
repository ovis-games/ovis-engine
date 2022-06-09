#pragma once

#include "ovis/utils/result.hpp"
#include "ovis/vm/parse_script_error.hpp"
#include <map>
#include <vector>

namespace ovis {

class VirtualMachine;

extern VirtualMachine* main_vm;

void InitializeMainVM();

struct LoadModuleError {
  std::map<std::string, std::vector<ParseScriptError>> script_errors;
};

Result<LoadModuleError> LoadGameModule();

}  // namespace ovis

