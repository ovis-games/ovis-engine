#include <ovis/core/scripting.hpp>

namespace ovis {

namespace {

double add(double x, double y) { return x + y; }
double subtract(double x, double y) { return x - y; }
double multiply(double x, double y) { return x * y; }
double divide(double x, double y) { return x / y; }
double negate(double x) { return -x; }

}

#define OVIS_REGISTER_FUNCTION(func) RegisterFunction<decltype(func), func>(#func);
#define OVIS_REGISTER_FUNCTION_WITH_NAME(func, identifier) RegisterFunction<decltype(func), func>(identifier);

ScriptContext::ScriptContext() {
  OVIS_REGISTER_FUNCTION(add);
  OVIS_REGISTER_FUNCTION(subtract);
  OVIS_REGISTER_FUNCTION(multiply);
  OVIS_REGISTER_FUNCTION(divide);
  OVIS_REGISTER_FUNCTION(negate);
}

void ScriptContext::RegisterFunction(std::string_view identifier, ScriptFunctionPointer function, std::vector<ScriptType> inputs,
                      std::vector<ScriptType> outputs) {
  functions_.insert(std::make_pair(identifier, ScriptFunction{function, inputs, outputs}));
}

const ScriptFunction* ScriptContext::GetFunction(std::string_view identifier) {
  const auto function = functions_.find(identifier);
  if (function == functions_.end()) {
    return nullptr;
  } else {
    return &function->second;
  }
}

std::variant<ScriptError, std::vector<ScriptVariable>> ScriptContext::Execute(std::string_view function_identifier,
                                                                              std::span<ScriptVariable> inputs) {
  const auto function = functions_.find(function_identifier);

  if (function == functions_.end()) {
    return ScriptError{};
  }

  if (function->second.inputs.size() != inputs.size()) {
    return ScriptError{};
  }

  std::vector<ScriptVariable> outputs(function->second.outputs.size());
  function->second.function(inputs, outputs);

  return outputs;
}

}  // namespace ovis
