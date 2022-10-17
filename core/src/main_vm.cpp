#include "ovis/core/main_vm.hpp"

#include "ovis/core/vm_bindings.hpp"
#include "ovis/vm/script_parser.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {
VirtualMachine* main_vm;

void InitializeMainVM() {
  main_vm = new VirtualMachine();

  main_vm->RegisterTypeAttribute("SceneObjectComponent", "Core", true);

  for (const auto& binding : VirtualMachineBinding::bindings()) {
    if (!main_vm->IsModuleRegistered(binding.module_name)) {
      main_vm->RegisterModule(binding.module_name);
    }
    binding.register_function(main_vm, binding.module_name);
  }
}

Result<void, ParseScriptErrors> LoadScriptModule(std::string_view name, AssetLibrary* asset_library) {
  assert(main_vm);

  if (main_vm->IsModuleRegistered(name)) {
    main_vm->DeregisterModule(name);
  }

  ScriptParser parser(main_vm, name);
  for (const auto& asset_id : asset_library->GetAssetsWithType("script")) {
    const auto script_asset_content = asset_library->LoadAssetJsonFile(asset_id, "json");
    assert(script_asset_content);
    parser.AddScript(*script_asset_content, asset_id);
  }

  if (parser.Parse()) {
    return Success;
  } else {
    return parser.errors();
  }
}

}  // namespace ovis
