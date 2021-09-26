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
  
  val inputs = val::object();
  for (const auto& input : function->inputs) {
    inputs.set(input.identifier, val(global_script_context()->GetType(input.type)->name));
  }
  function_info.set("inputs", inputs);

  val outputs = val::object();
  for (const auto& output : function->outputs) {
    outputs.set(output.identifier, val(global_script_context()->GetType(output.type)->name));
  }
  function_info.set("outputs", outputs);

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

std::optional<ScriptChunk> current_script;
val SetScript(const std::string& script) {
  current_script.reset();

  auto load_result = ScriptChunk::Load(json::parse(script));
  val result = val::object();
  if (std::holds_alternative<ScriptError>(load_result)) {
    const ScriptError& error = std::get<ScriptError>(load_result);
    val result_error = val::object();
    result_error.set("action", val(error.action.string));
    result_error.set("message", val(error.message));
    result.set("error", result_error);
  } else {
    current_script.emplace(std::move(std::get<ScriptChunk>(load_result)));
  }

  return result;
}

val RunScript() {
  val result = val::object();

  if (!current_script.has_value()) {
    val result_error = val::object();
    result_error.set("message", "Script has errors");
    result.set("error", result_error);
  } else {
    const ScriptFunctionResult function_result = current_script->Execute({});
    if (function_result.error.has_value()) {
      val result_error = val::object();
      result_error.set("action", val(function_result.error->action.string));
      result_error.set("message", val(function_result.error->message));
      result.set("error", result_error);
    }
  }

  return result;
}

EMSCRIPTEN_BINDINGS(script_compiler_module) {
  function("scriptCompilerGetFunctios", &GetFunctions);
  function("scriptCompilerGetFunctionInfo", &GetFunctionInfo);
  function("scriptCompilerGetDocumentation", &GetDocumentation);
  function("scriptCompilerSetScript", &SetScript);
  function("scriptCompilerRunScript", &RunScript);
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
