#pragma once

#include <any>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <variant>

#include <ovis/utils/serialize.hpp>
#include <ovis/utils/range.hpp>

namespace ovis {

struct ScriptType {};

struct ScriptVariableDefinition {
  ScriptType type;
  std::string identifier;
};

struct ScriptVariable {
  std::string type;
  std::any value;
};

using ScriptFunctionParameters = std::vector<ScriptVariable>;

using ScriptFunctionPointer = void (*)(std::span<ScriptVariable> input, std::span<ScriptVariable> output);

struct ScriptFunction {
  ScriptFunctionPointer function;
  std::vector<ScriptVariableDefinition> inputs;
  std::vector<ScriptVariableDefinition> outputs;

  size_t GetInputIndex(std::string_view input_identifier) const {
    for (const auto& input : IndexRange(outputs)) {
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
};

struct ScriptError {};

class ScriptChunk;

class ScriptContext {
 public:
  ScriptContext();

  void RegisterFunction(std::string_view identifier, ScriptFunctionPointer function,
                        std::span<const ScriptVariableDefinition> inputs,
                        std::span<const ScriptVariableDefinition> outputs);

  template <typename T, T FUNCTION>
  void RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names = {},
                        std::vector<std::string> outputs_names = {});

  const ScriptFunction* GetFunction(std::string_view identifier);

  std::variant<ScriptError, std::vector<ScriptVariable>> Execute(std::string_view function_identifier,
                                                                 std::span<ScriptVariable> arguments);

  std::span<ScriptVariable> PushStack(size_t count) {
    if (stack_.size() + count > stack_.capacity()) {
      throw std::runtime_error("Script stack overflow");
    }
    const size_t begin = stack_.size();
    stack_.resize(begin + count);

    return {stack_.data() + begin, count};
  }

  void PopStack(size_t count) {
    if (count > stack_.size()) {
      throw std::runtime_error("Script stack underflow");
    }
    stack_.resize(stack_.size() - count);
  }

  ScriptVariable Get(size_t offset) {
    return stack_[stack_.size() - 1 - offset];
  }

 private:
  std::map<std::string, ScriptFunction, std::less<>> functions_;
  std::vector<ScriptVariable> stack_;
};

static ScriptContext global_script_context;

class ScriptEnvironment {
 public:
 private:
  ScriptEnvironment* parent = nullptr;
  std::map<std::string, ScriptVariable> variables_;
};

class ScriptChunk : public Serializable {
  struct Instruction {
    ScriptFunction function;
    std::vector<std::variant<std::string, int, ScriptVariable>> inputs;
  };

 public:
  bool Deserialize(const json& serialized_chunk) override;
  json Serialize() const override;
  std::variant<ScriptError, std::vector<ScriptVariable>> Execute();

 private:
  ScriptContext* context_;
  std::vector<Instruction> instructions_;
};

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType(ArgumentTypes...)> {
  using FunctionType = ReturnType (*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes);
  static constexpr std::array<ScriptType, INPUT_COUNT> INPUT_TYPES;
  static constexpr auto OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;
  static constexpr std::array<ScriptType, OUTPUT_COUNT> OUTPUT_TYPES;

  template <FunctionType FUNCTION>
  static void Execute(std::span<ScriptVariable> inputs, std::span<ScriptVariable> outputs) {
    std::tuple<ArgumentTypes...> args;
    FillArgumentTuple<0>(&args, inputs);
    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, args);
    } else {
      const auto result = std::apply(FUNCTION, args);
      outputs[0].value = result;
    }
  }

  template <size_t N>
  static void FillArgumentTuple(std::tuple<ArgumentTypes...>* arguments, std::span<ScriptVariable> inputs) {
    if constexpr (N == sizeof...(ArgumentTypes)) {
      return;
    } else {
      std::get<N>(*arguments) =
          std::any_cast<typename std::tuple_element<N, std::tuple<ArgumentTypes...>>::type>(inputs[N].value);
      FillArgumentTuple<N + 1>(arguments, inputs);
    }
  }
};

template <typename FunctionType, FunctionType FUNCTION>
void ScriptContext::RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names, std::vector<std::string> outputs_names) {
  using Wrapper = FunctionWrapper<FunctionType>;

  std::vector<ScriptVariableDefinition> inputs;
  inputs.reserve(Wrapper::INPUT_TYPES.size());
  for (const auto& type : IndexRange(Wrapper::INPUT_TYPES)) {
    inputs.push_back({type.value(), type.index() < outputs_names.size() ? outputs_names[type.index()] : std::to_string(type.index())});
  }

  std::vector<ScriptVariableDefinition> outputs;
  inputs.reserve(Wrapper::OUTPUT_TYPES.size());
  for (const auto& type : IndexRange(Wrapper::OUTPUT_TYPES)) {
    outputs.push_back({type.value(), type.index() < outputs_names.size() ? outputs_names[type.index()] : std::to_string(type.index())});
  }

  RegisterFunction(identifier, &Wrapper::template Execute<FUNCTION>, inputs, outputs);
}

}  // namespace ovis

