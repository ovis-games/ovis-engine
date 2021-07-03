#pragma once

#include <any>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <variant>

#include <ovis/utils/serialize.hpp>

namespace ovis {

struct ScriptType {
};

struct ScriptVariable {
  std::string type;
  std::any value;
};

using ScriptFunctionParameters = std::vector<ScriptVariable>;

using ScriptFunctionPointer = void (*)(std::span<ScriptVariable> input, std::span<ScriptVariable> output);

struct ScriptFunction {
  ScriptFunctionPointer function;
  std::vector<ScriptType> inputs;
  std::vector<ScriptType> outputs;
};

struct ScriptError {};

class ScriptChunk;

class ScriptContext {
 public:
  ScriptContext();

  void RegisterFunction(std::string_view identifier, ScriptFunctionPointer function, std::vector<ScriptType> inputs,
                        std::vector<ScriptType> outputs);

  template <typename T, T FUNCTION>
  void RegisterFunction(std::string_view identifier);

  const ScriptFunction* GetFunction(std::string_view identifier);

  std::variant<ScriptError, std::vector<ScriptVariable>> Execute(std::string_view function_identifier,
                                                                 std::span<ScriptVariable> arguments);
  std::variant<ScriptError, std::vector<ScriptVariable>> Execute(const ScriptChunk& chunk);

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
  struct Action {
    ScriptFunction function;
    std::vector<std::variant<std::string, int, ScriptVariable>> parameters;
  };

 public:
  bool Deserialize(const json& serialized_chunk) override;
  json Serialize() const override;

 private:
  std::vector<Action> actions_;
};

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType(ArgumentTypes...)> {
  using FunctionType = ReturnType(*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes);
  static const std::vector<ScriptType> INPUT_TYPES;
  static constexpr auto OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;
  static const std::vector<ScriptType> OUTPUT_TYPES;

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
      std::get<N>(*arguments) = std::any_cast<typename std::tuple_element<N, std::tuple<ArgumentTypes...>>::type>(inputs[N].value);
      FillArgumentTuple<N + 1>(arguments, inputs);
    }
  }
};

template <typename ReturnType, typename... ArgumentTypes>
const std::vector<ScriptType> FunctionWrapper<ReturnType(ArgumentTypes...)>::INPUT_TYPES(sizeof...(ArgumentTypes));

template <typename ReturnType, typename... ArgumentTypes>
const std::vector<ScriptType> FunctionWrapper<ReturnType(ArgumentTypes...)>::OUTPUT_TYPES(FunctionWrapper<ReturnType(ArgumentTypes...)>::OUTPUT_COUNT);

template <typename FunctionType, FunctionType FUNCTION>
void ScriptContext::RegisterFunction(std::string_view identifier) {
  using Wrapper = FunctionWrapper<FunctionType>;
  RegisterFunction(identifier, &Wrapper::template Execute<FUNCTION>, Wrapper::INPUT_TYPES, Wrapper::OUTPUT_TYPES);
}

}  // namespace ovis

