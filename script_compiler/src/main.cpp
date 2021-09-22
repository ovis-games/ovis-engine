#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/rendering/clear_pass.hpp>
#include <ovis/application/application.hpp>
#include <ovis/application/window.hpp>

using namespace ovis;
// using namespace ovis::editor;
using namespace emscripten;

emscripten::val GetFunctionInfo(const std::string& function_identifier) {
  const ScriptFunction* function = global_script_context()->GetFunction(function_identifier);
  if (function == nullptr) {
    return emscripten::val::null();
  }

  emscripten::val function_info = emscripten::val::object();
  function_info.set("text", val(function->text));
  function_info.set("description", val(function->description));

  return function_info;
}

emscripten::val GetFunctions() {
  val functions = emscripten::val::object();

  for (const auto& function_identifier : global_script_context()->function_identifiers()) {
    const auto function = GetFunctionInfo(function_identifier);
    if (function != val::null()) {
      functions.set(function_identifier, function);
    }
  }

  return functions;

}

emscripten::val GetDocumentation() {
  val doc = val::object();
  doc.set("functions", GetFunctions());
  return doc;

}

EMSCRIPTEN_BINDINGS(script_compiler_module) {
  function("scriptCompilerGetFunctios", &GetFunctions);
  function("scriptCompilerGetFunctionInfo", &GetFunctionInfo);
  function("scriptCompilerGetDocumentation", &GetDocumentation);
}


int main(int argc, char* argv[]) {
  Log::AddListener(ConsoleLogger);

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  global_script_context()->LoadDocumentation();

  // In theory not necessary, but i'll keep it here just for now
  Run();

  Quit();
}
