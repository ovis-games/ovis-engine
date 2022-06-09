#include "ovis/core/main_vm.hpp"

#include "ovis/core/asset_library.hpp"
#include "ovis/core/vm_bindings.hpp"
#include "ovis/vm/script_parser.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {
VirtualMachine* main_vm;

void InitializeMainVM() {
  main_vm = new VirtualMachine();

  for (const auto& binding : VirtualMachineBinding::bindings()) {
    auto module = main_vm->GetModule(binding.module_name);
    if (!module) {
      module = main_vm->RegisterModule(binding.module_name);
    }
    assert(module);
    binding.register_function(module.get());
  }
}

Result<LoadModuleError> LoadGameModule() {
  assert(main_vm);

  if (main_vm->GetModule("Game")) {
    main_vm->DeregisterModule("Game");
  }

  const auto game_module = main_vm->RegisterModule("Game");
  LoadModuleError error;
  for (const auto& asset_id : GetApplicationAssetLibrary()->GetAssetsWithType("script")) {
    const auto script_asset_content = GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id, "script");
    assert(script_asset_content);

    const auto parse_result = ParseScript(main_vm, json::parse(*script_asset_content));
    if (!parse_result) {
      error.script_errors[asset_id] = parse_result.error();
    }
  }
  
}

}  // namespace ovis
