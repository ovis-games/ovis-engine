#pragma once

#include <optional>
#include <vector>

#include <ovis/core/script_error.hpp>
#include <ovis/core/script_type.hpp>

namespace ovis {

class ScriptContext;

class ScriptFunction {
 public:
  ScriptFunction(std::vector<ScriptValueDefinition> inputs, std::vector<ScriptValueDefinition> outputs)
      : inputs(std::move(inputs)), outputs(std::move(outputs)) {}
  virtual ~ScriptFunction() = default;

  size_t GetInputIndex(std::string_view input_identifier) const {
    for (const auto& input : IndexRange(inputs)) {
      if (input->identifier == input_identifier) {
        return input.index();
      }
    }

    return -1;
  }
  size_t GetOutputIndex(std::string_view output_identifier) const {
    for (const auto& output : IndexRange(outputs)) {
      if (output->identifier == output_identifier) {
        return output.index();
      }
    }

    return -1;
  }

 public:
  std::vector<ScriptValueDefinition> inputs;
  std::vector<ScriptValueDefinition> outputs;
  std::string text;
  std::string description;
};

using ScriptFunctionPointer = std::optional<ScriptError> (*)(ScriptContext* context, int input_count, int output_count);
class NativeScriptFunction : public ScriptFunction {
 public:
  ScriptFunctionPointer function;

  NativeScriptFunction(std::vector<ScriptValueDefinition> inputs, std::vector<ScriptValueDefinition> outputs,
                       ScriptFunctionPointer pointer)
      : ScriptFunction(inputs, outputs), function(pointer) {}
};

}  // namespace ovis
