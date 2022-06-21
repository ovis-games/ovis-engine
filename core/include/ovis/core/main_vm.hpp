#pragma once

#include "ovis/core/asset_library.hpp"
#include <map>
#include <vector>

#include "ovis/utils/result.hpp"
#include "ovis/vm/parse_script_error.hpp"

namespace ovis {

class VirtualMachine;

extern VirtualMachine* main_vm;

void InitializeMainVM();

struct LoadModuleError {
  std::map<std::string, std::vector<ParseScriptError>> script_errors;
};

Result<void, LoadModuleError> LoadScriptModule(std::string_view name, AssetLibrary* asset_library);

}  // namespace ovis

