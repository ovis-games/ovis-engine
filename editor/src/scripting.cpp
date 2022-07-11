#include "ovis/editor/scripting.hpp"

#include <emscripten/bind.h>

#include "ovis/vm/script_parser.hpp"
#include "ovis/core/main_vm.hpp"

namespace ovis {

namespace {

emscripten::val error_callback = emscripten::val::undefined();

emscripten::val LocationToJSValue(const ScriptErrorLocation& location) {
  auto value = emscripten::val::object();

  value.set("jsonPath", location.json_path);
  value.set("scriptName", location.script_name);

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
    return emscripten::val::array();
  } else {
    auto js_errors = TransformRange(parse_result.error(), [](const auto& error) { return ErrorToJSValue(error); });
    return emscripten::val::array(js_errors.begin(), js_errors.end());
  }
}

bool parseGameModuleScripts() {
  const auto module_load_result = LoadScriptModule("Game", GetApplicationAssetLibrary());

  if (error_callback.isTrue()) {
    error_callback(ErrorsResultToJSValue(module_load_result));
  }

  return module_load_result;
}

void setErrorCallback(const emscripten::val& new_error_callback) {
  error_callback = new_error_callback;
}

EMSCRIPTEN_BINDINGS(editor_module) {
  emscripten::function("parseGameModuleScripts", parseGameModuleScripts);
  emscripten::function("setErrorCallback", setErrorCallback);
}

}

}
