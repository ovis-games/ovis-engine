#include "ovis/editor/scripting.hpp"

#include <emscripten/bind.h>

#include "ovis/vm/script_parser.hpp"
#include "ovis/core/main_vm.hpp"

namespace ovis {

namespace {

emscripten::val error_callback = emscripten::val::undefined();

emscripten::val LocationToJSValue(const ScriptErrorLocation& location) {
  auto value = emscripten::val::object();

  value.set("path", location.json_path);
  value.set("asset", location.script_name);

  return value;
}

emscripten::val ErrorToJSValue(const ParseScriptError& error) {
  auto value = emscripten::val::object();

  value.set("message", error.message);
  if (error.location) {
    value.set("location", LocationToJSValue(*error.location));
  }

  return value;
}

emscripten::val ErrorsResultToJSValue(const Result<void, ParseScriptErrors>& parse_result) {
  if (parse_result) {
    return emscripten::val::object();
  } else {
    auto errors = emscripten::val::object();
    for (const auto& error : parse_result.error()) {
      assert(error.location.has_value());
      if (!errors.hasOwnProperty(error.location->script_name.c_str())) {
        errors.set(error.location->script_name, emscripten::val::array());
      }
      errors[error.location->script_name].call<void>("push", ErrorToJSValue(error));
    }
    return errors;
  }
}

emscripten::val CreateTypeReflection(TypeId type_id) {
  const Type* type = main_vm->GetType(type_id);
  if (!type) {
    return emscripten::val::undefined();
  }

  auto type_reflection = emscripten::val::object();

  return type_reflection;
}

emscripten::val CreateBuiltInTypesReflection() {
  auto built_in_types = emscripten::val::object();

  built_in_types.set("Number", CreateTypeReflection(main_vm->GetTypeId("Number")));
  built_in_types.set("String", CreateTypeReflection(main_vm->GetTypeId("String")));
  built_in_types.set("Boolean", CreateTypeReflection(main_vm->GetTypeId("Boolean")));

  return built_in_types;
}

emscripten::val CreateModuleTypesReflection(Module* module) {
  auto types = emscripten::val::object();

  for (const auto& type_id : module->registered_type_ids()) {
    const auto type = main_vm->GetType(type_id);
    types.set(std::string(type->name()), CreateTypeReflection(type_id));
  }

  return types;
}

emscripten::val CreateModuleReflection(Module* module) {
  auto module_reflection = emscripten::val::object();

  module_reflection.set("types", CreateModuleTypesReflection(module));

  return module_reflection;
}

emscripten::val CreateModulesReflection() {
  auto modules = emscripten::val::object();

  for (const auto& module : main_vm->registered_modules()) {
    modules.set(std::string(module->name()), CreateModuleReflection(module));
  }

  return modules;
}

emscripten::val CreateReflection() {
  auto reflection = emscripten::val::object();

  reflection.set("builtInTypes", CreateBuiltInTypesReflection());
  reflection.set("modules", CreateModulesReflection());

  return reflection;
}

emscripten::val parseGameModuleScripts() {
  const auto module_load_result = LoadScriptModule("Game", GetApplicationAssetLibrary());

  auto result = emscripten::val::object();
  result.set("reflection", CreateReflection());
  result.set("errors", ErrorsResultToJSValue(module_load_result));
  return result;
}

void setErrorCallback(const emscripten::val& new_error_callback) {
  error_callback = new_error_callback;
}

EMSCRIPTEN_BINDINGS(editor_module) {
  emscripten::function("parseGameModuleScripts", parseGameModuleScripts);
}

}

}
