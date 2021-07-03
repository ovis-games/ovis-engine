#include <ovis/core/scripting.hpp>

namespace ovis {

void ScriptContext::RegisterFunction(std::string_view identifier, ScriptFunctionPointer function, int input_count,
                      int output_count) {
  functions_.insert(std::make_pair(identifier, ScriptFunction{ function, input_count, output_count }));
}

std::variant<ScriptError, std::vector<ScriptVariable>> ScriptContext::Execute(std::string_view function_identifier,
                                                                              std::span<ScriptVariable> arguments) {
  const auto function = functions_.find(function_identifier);

  if (function == functions_.end()) {
    return ScriptError{};
  }

  if (function->second.input_count != arguments.size()) {
    return ScriptError{};
  }

  std::vector<ScriptVariable> outputs(function->second.output_count);
  function->second.function(arguments, outputs);

  return outputs;
}

}  // namespace ovis
