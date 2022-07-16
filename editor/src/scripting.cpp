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

emscripten::val parseGameModuleScripts() {
  const auto module_load_result = LoadScriptModule("Game", GetApplicationAssetLibrary());
  return ErrorsResultToJSValue(module_load_result);
}

void setErrorCallback(const emscripten::val& new_error_callback) {
  error_callback = new_error_callback;
}

EMSCRIPTEN_BINDINGS(editor_module) {
  emscripten::function("parseGameModuleScripts", parseGameModuleScripts);
}

}

}
