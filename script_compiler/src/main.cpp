#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

#include "ovis/application/application.hpp"
#include "ovis/application/window.hpp"
#include "ovis/core/asset_library.hpp"
#include "ovis/core/main_vm.hpp"

#include "ovis/utils/log.hpp"

using namespace ovis;
// using namespace ovis::editor;
using namespace emscripten;

extern "C" {

void EMSCRIPTEN_KEEPALIVE loadGameModule() {
  const auto result = ovis::LoadScriptModule("Game", GetApplicationAssetLibrary());
  if (!result) {
    for (const auto& [asset, errors] : result.error().script_errors) {
      for (const auto& error : errors) {
        LogE("{}", error.message);
      }
    }
  }
}

}

// emscripten::val log_callback = emscripten::val::undefined();
// void SetLogCallback(emscripten::val callback) {
//   log_callback = callback;
// }

// val GetTypeInfo(safe_ptr<vm::Type> type) {
//   if (type == nullptr) {
//     return val::null();
//   }

//   val type_info = val::object();
//   type_info.set("name", val(std::string(type->name())));
//   assert(type->module() != nullptr);
//   type_info.set("module", val(std::string(type->module()->name())));
//   return type_info;
// }

// val getdeclarationinfo(const vm::function::valuedeclaration& declaration) {
//   val declaration_info = val::object();
//   declaration_info.set("name", val(declaration.name));
//   declaration_info.set("type", gettypeinfo(declaration.type));
//   return declaration_info ;
// }

// emscripten::val getfunctioninfo(const vm::function& function) {
//   emscripten::val function_info = emscripten::val::object();
//   function_info.set("name", val(std::string(function.name())));
//   function_info.set("text", val(std::string(function.text())));
//   // function_info.set("description", val(function->description));
  
//   val inputs = val::array();
//   for (const auto& input : function.inputs()) {
//     inputs.call<void>("push", getdeclarationinfo(input));
//   }
//   function_info.set("inputs", inputs);

//   val outputs = val::array();
//   for (const auto& output : function.outputs()) {
//     outputs.call<void>("push", getdeclarationinfo(output));
//   }
//   function_info.set("outputs", outputs);

//   return function_info;
// }

// emscripten::val getfunctions(const vm::module& module) {
//   val functions = emscripten::val::object();

//   for (const auto& function : module.functions()) {
//     functions.set(std::string(function.name()), getfunctioninfo(function));
//   }

//   return functions;
// }

// emscripten::val gettypeinfo(const vm::type& type) {
//   emscripten::val type_info = emscripten::val::object();

//   return type_info;
// }

// emscripten::val gettypes(const vm::module& module) {
//   val types = emscripten::val::object();

//   for (const auto& type : module.types()) {
//     types.set(std::string(type.name()), gettypeinfo(type));
//   }

//   return types;
// }

// val getmoduleinfo(const vm::module& module) {
//   val module_info = val::object();

//   module_info.set("functions", getfunctions(module));
//   module_info.set("types", gettypes(module));

//   return module_info;
// }

// val getmodules() {
//   val modules = val::object();
//   for (const auto& module : vm::module::registered_modules()) {
//     modules.set(std::string(module.name()), getmoduleinfo(module));
//   }
//   return modules;
// }

// emscripten::val getdocumentation() {
//   val doc = val::object();

//   doc.set("modules", getmodules());
//   // doc.set("types", gettypes());

//   return doc;
// }

// std::optional<scriptfunction> script_function;
// val setscript(const std::string& script) {
//   scriptfunctionparser parser(json::parse(script));

//   val result = val::object();
//   val errors = val::array();
//   for (const auto& error : parser.errors) {
//     val result_error = val::object();
//     result_error.set("path", val(error.path));
//     result_error.set("message", val(error.message));
//     errors.call<void>("push", result_error);
//   }
//   result.set("errors", errors);
//   if (parser.errors.size() == 0) {
//     script_function.emplace(parser);
//   }

//   return result;
// }

// val runscript() {
//   val result = val::object();

//   if (!script_function.has_value()) {
//     val result_error = val::object();
//     result_error.set("message", "script has errors");
//     result.set("error", result_error);
//   } else {
//     script_function->call();
//     // const auto function_result = chunk->call({});
//     // if (std::holds_alternative<scripterror>(function_result)) {
//     //   const scripterror& error = std::get<scripterror>(function_result);
//     //   val result_error = val::object();
//     //   // todo: temporary hackfix, use the json path as action reference
//     //   result_error.set("action", val("/actions" + error.action.string));
//     //   result_error.set("message", val(error.message));
//     //   result.set("error", result_error);
//     //   global_script_context()->printdebuginfo();
//     // }
//   }

//   return result;
// }

// EMSCRIPTEN_BINDINGS(script_compiler_module) {
//   function("scriptCompilerGetFunctions", &GetFunctions);
//   function("scriptCompilerGetDocumentation", &GetDocumentation);
//   function("scriptCompilerSetScript", &SetScript);
//   function("scriptCompilerRunScript", &RunScript);
//   function("scriptCompilerSetLogCallback", &SetLogCallback);
// }


int main(int argc, char* argv[]) {
  // Log::AddListener(ConsoleLogger);
  // Log::AddListener([](LogLevel level, const std::string& text) {
  //   if (log_callback.typeOf() == emscripten::val("function")) {
  //     log_callback(emscripten::val(static_cast<int>(level)), emscripten::val(text));
  //   }
  // });

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  // In theory not necessary, but i'll keep it here just for now
  Run();

  Quit();
}
