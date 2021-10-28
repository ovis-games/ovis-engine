#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_parser.hpp>
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

val GetTypeInfo(safe_ptr<vm::Type> type) {
  if (type == nullptr) {
    return val::null();
  }

  val type_info = val::object();
  type_info.set("name", val(std::string(type->name())));
  assert(type->module() != nullptr);
  type_info.set("module", val(std::string(type->module()->name())));
  return type_info;
}

val GetDeclarationInfo(const vm::Function::ValueDeclaration& declaration) {
  val declaration_info = val::object();
  declaration_info.set("name", val(declaration.name));
  declaration_info.set("type", GetTypeInfo(declaration.type));
  return declaration_info ;
}

emscripten::val GetFunctionInfo(const vm::Function& function) {
  emscripten::val function_info = emscripten::val::object();
  function_info.set("name", val(std::string(function.name())));
  function_info.set("text", val(std::string(function.text())));
  // function_info.set("description", val(function->description));
  
  val inputs = val::array();
  for (const auto& input : function.inputs()) {
    inputs.call<void>("push", GetDeclarationInfo(input));
  }
  function_info.set("inputs", inputs);

  val outputs = val::array();
  for (const auto& output : function.outputs()) {
    outputs.call<void>("push", GetDeclarationInfo(output));
  }
  function_info.set("outputs", outputs);

  return function_info;
}

emscripten::val GetFunctions(const vm::Module& module) {
  val functions = emscripten::val::object();

  for (const auto& function : module.functions()) {
    functions.set(std::string(function.name()), GetFunctionInfo(function));
  }

  return functions;
}

emscripten::val GetTypeInfo(const vm::Type& type) {
  emscripten::val type_info = emscripten::val::object();

  return type_info;
}

emscripten::val GetTypes(const vm::Module& module) {
  val types = emscripten::val::object();

  for (const auto& type : module.types()) {
    types.set(std::string(type.name()), GetTypeInfo(type));
  }

  return types;
}

val GetModuleInfo(const vm::Module& module) {
  val module_info = val::object();

  module_info.set("functions", GetFunctions(module));
  module_info.set("types", GetTypes(module));

  return module_info;
}

val GetModules() {
  val modules = val::object();
  for (const auto& module : vm::Module::registered_modules()) {
    modules.set(std::string(module.name()), GetModuleInfo(module));
  }
  return modules;
}

emscripten::val GetDocumentation() {
  val doc = val::object();

  doc.set("modules", GetModules());
  // doc.set("types", GetTypes());

  return doc;
}

std::optional<ScriptFunction> script_function;
val SetScript(const std::string& script) {
  ScriptFunctionParser parser(json::parse(script));

  val result = val::object();
  val errors = val::array();
  for (const auto& error : parser.errors) {
    val result_error = val::object();
    result_error.set("path", val(error.path));
    result_error.set("message", val(error.message));
    errors.call<void>("push", result_error);
  }
  result.set("errors", errors);
  if (parser.errors.size() == 0) {
    script_function.emplace(parser);
  }

  return result;
}

val RunScript() {
  val result = val::object();

  if (!script_function.has_value()) {
    val result_error = val::object();
    result_error.set("message", "Script has errors");
    result.set("error", result_error);
  } else {
    script_function->Call();
    // const auto function_result = chunk->Call({});
    // if (std::holds_alternative<ScriptError>(function_result)) {
    //   const ScriptError& error = std::get<ScriptError>(function_result);
    //   val result_error = val::object();
    //   // TODO: temporary hackfix, use the json path as action reference
    //   result_error.set("action", val("/actions" + error.action.string));
    //   result_error.set("message", val(error.message));
    //   result.set("error", result_error);
    //   global_script_context()->PrintDebugInfo();
    // }
  }

  return result;
}

EMSCRIPTEN_BINDINGS(script_compiler_module) {
  function("scriptCompilerGetFunctions", &GetFunctions);
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

  // In theory not necessary, but i'll keep it here just for now
  Run();

  Quit();
}
