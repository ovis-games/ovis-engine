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


emscripten::val log_callback = emscripten::val::undefined();
void SetLogCallback(emscripten::val callback) {
  log_callback = callback;
}

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

emscripten::val GetTypeInfo(const std::string& type_name) {
  const auto type = global_script_context()->GetType(type_name);

  if (type == nullptr) {
    return emscripten::val::null();
  }

  emscripten::val type_info = emscripten::val::object();
  if (type->base_type_id != SCRIPT_TYPE_UNKNOWN) {
    const auto base_type = global_script_context()->GetType(type->base_type_id);
    type_info.set("base", emscripten::val(base_type->name));
  }

  return type_info;
}

emscripten::val GetTypes() {
  val types = emscripten::val::object();

  for (const auto& type_name : global_script_context()->type_names()) {
    const auto type = GetTypeInfo(type_name);
    if (type != val::null()) {
      types.set(type_name, type_name);
    }
  }

  return types;
}

emscripten::val GetDocumentation() {
  val doc = val::object();
  doc.set("functions", GetFunctions());
  doc.set("types", GetTypes());
  return doc;

}

std::optional<ScriptChunk> chunk;
val SetScript(const std::string& script) {
  chunk.reset();

  val result = val::object();
  auto chunk_or_error = ScriptChunk::Load(global_script_context(), json::parse(script));
  if (std::holds_alternative<ScriptError>(chunk_or_error)) {
    const ScriptError& error = std::get<ScriptError>(chunk_or_error);
    val result_error = val::object();
    // TODO: temporary hackfix, use the json path as action reference
    result_error.set("action", val("/actions" + error.action.string));
    result_error.set("message", val(error.message));
    result.set("error", result_error);
  } else {
    chunk = std::move(std::get<ScriptChunk>(chunk_or_error));
    result.set("instructions", val(chunk->GetInstructionsAsText()));
  }

  return result;
}

val RunScript() {
  val result = val::object();

  if (!chunk.has_value()) {
    val result_error = val::object();
    result_error.set("message", "Script has errors");
    result.set("error", result_error);
  } else {
    const auto function_result = chunk->Call({});
    if (std::holds_alternative<ScriptError>(function_result)) {
      const ScriptError& error = std::get<ScriptError>(function_result);
      val result_error = val::object();
      // TODO: temporary hackfix, use the json path as action reference
      result_error.set("action", val("/actions" + error.action.string));
      result_error.set("message", val(error.message));
      result.set("error", result_error);
      global_script_context()->PrintDebugInfo();
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
  function("scriptCompilerSetLogCallback", &SetLogCallback);
}


int main(int argc, char* argv[]) {
  // Log::AddListener(ConsoleLogger);
  Log::AddListener([](LogLevel level, const std::string& text) {
    if (log_callback.typeOf() == emscripten::val("function")) {
      log_callback(emscripten::val(static_cast<int>(level)), emscripten::val(text));
    }
  });

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  global_script_context()->LoadDocumentation();

  // In theory not necessary, but i'll keep it here just for now
  Run();

  Quit();
}
