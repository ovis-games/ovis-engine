#pragma once

#include <map>
#include <vector>

#include "ovis/utils/result.hpp"
#include "ovis/vm/parse_script_error.hpp"
#include "ovis/core/asset_library.hpp"

namespace ovis {

class VirtualMachine;

extern VirtualMachine* main_vm;

void InitializeMainVM();

Result<void, ParseScriptErrors> LoadScriptModule(std::string_view name, AssetLibrary* asset_library);

}  // namespace ovis

