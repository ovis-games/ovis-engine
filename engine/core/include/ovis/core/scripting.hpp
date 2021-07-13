#pragma once

#include <any>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <typeindex>
#include <variant>
#include <limits>

#include <SDL_assert.h>

#include <ovis/utils/range.hpp>
#include <ovis/utils/serialize.hpp>

namespace ovis {

class ScriptContext;

using ScriptTypeId = size_t;
constexpr ScriptTypeId SCRIPT_TYPE_UNKNOWN = std::numeric_limits<ScriptTypeId>::max();

struct ScriptType {
  ScriptType(ScriptTypeId id, std::string_view name, bool reference_type)
      : id(id), name(name), reference_type(reference_type) {}

  ScriptTypeId id;
  std::string name;
  bool reference_type;
};

struct ScriptVariableDefinition {
  ScriptTypeId type;
  std::string identifier;
};

struct ScriptVariable {
  ScriptTypeId type;
  std::any value;
};

using ScriptFunctionParameters = std::vector<ScriptVariable>;

using ScriptFunctionPointer = void (*)(ScriptContext* context, int input_count, int output_count);

struct ScriptFunction {
  ScriptFunctionPointer function;
  std::vector<ScriptVariableDefinition> inputs;
  std::vector<ScriptVariableDefinition> outputs;

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
};

struct ScriptError {};

struct ScriptFunctionResult {
  std::vector<ScriptVariable> output_values;
  std::optional<ScriptError> error;
};

struct ScriptParseError {
  size_t action_id;
  std::string message;
};

class ScriptChunk;

class ScriptContext {
 public:
  ScriptContext();

  template <typename T>
  ScriptType* RegisterType(std::string name) {
    ScriptTypeId id = types_.size();
    SDL_assert(type_names_.find(name) == type_names_.end());
    SDL_assert(type_indices_.find(typeid(T)) == type_indices_.end());

    types_.emplace_back(id, name, false);
    type_names_.insert(std::make_pair(name, id));
    type_indices_.insert(std::make_pair(std::type_index(typeid(T)), id));

    return &types_.back();
  }

  ScriptTypeId GetTypeId(std::string_view name) {
    const auto it = type_names_.find(std::string(name));
    if (it == type_names_.end()) {
      return SCRIPT_TYPE_UNKNOWN;
    } else {
      return it->second;
    }
  }

  const ScriptType* GetType(std::string_view name) {
    const ScriptTypeId type_id = GetTypeId(name);
    if (type_id == SCRIPT_TYPE_UNKNOWN) {
      return nullptr;
    } else {
      return &types_[type_id];
    }
  }

  template <typename T>
  ScriptTypeId GetTypeId() {
    const auto it = type_indices_.find(typeid(T));
    if (it == type_indices_.end()) {
      return SCRIPT_TYPE_UNKNOWN;
    } else {
      return it->second;
    }
  }

  template <typename T>
  const ScriptType* GetType() {
    const ScriptTypeId type_id = GetTypeId<T>();
    if (type_id == SCRIPT_TYPE_UNKNOWN) {
      return nullptr;
    } else {
      return &types_[type_id];
    }
  }

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

  void AssignValue(int offset, ScriptVariable value) { GetValue(offset) = value; }

  template <typename T>
  void AssignValue(int offset, T&& new_value) {
    ScriptVariable& value = GetValue(offset);
    value.type = GetTypeId<T>();
    value.value = new_value;
  }

  void PushValue(ScriptVariable value) {
    if (stack_.size() >= stack_.capacity()) {
      std::runtime_error("Stack overflow!");
    }
    stack_.emplace_back(std::move(value));
  }

  template <typename T>
  void PushValue(T&& value) {
    ScriptVariable script_value{GetTypeId<T>(), value};
    PushValue(script_value);
  }

  template <typename T, typename... Ts>
  void PushValues(T&& value, Ts&&... values) {
    PushValue(std::forward<T>(value));
    if constexpr (sizeof...(Ts) > 0) {
      PushValues<Ts...>(std::forward<Ts>(values)...);
    }
  }

  void PopStack(size_t count) {
    if (count > stack_.size()) {
      throw std::runtime_error("Script stack underflow");
    }
    stack_.resize(stack_.size() - count);
  }

  ScriptVariable& GetValue(int offset) {
    SDL_assert(offset < 0);
    return stack_[stack_.size() + offset];
  }

  template <typename T>
  T GetValue(int offset) {
    const ScriptVariable& value = GetValue(offset);
    return std::any_cast<T>(value.value);
  }

  template <typename... T>
  std::tuple<T...> GetValues(int begin) {
    std::tuple<T...> values;
    FillValueTuple<0>(&values, begin);
    return values;
  }

  template <int N, typename... T>
  void FillValueTuple(std::tuple<T...>* values, int begin) {
    if constexpr (N == sizeof...(T)) {
      return;
    } else {
      std::get<N>(*values) = GetValue<std::tuple_element_t<N, std::tuple<T...>>>(begin + N);
      FillValueTuple<N + 1>(values, begin);
    }
  }

  std::span<ScriptVariable> GetRange(int begin, int end) {
    SDL_assert(begin < 0);
    SDL_assert(end <= 0);
    SDL_assert(begin <= end);
    SDL_assert(-begin <= stack_.size());
    SDL_assert(end - begin >= 0);
    return {&stack_[stack_.size() + begin], static_cast<size_t>(end - begin)};
  }

 private:
  std::map<std::string, ScriptFunction, std::less<>> functions_;
  std::vector<ScriptVariable> stack_;
  std::vector<ScriptType> types_;
  std::unordered_map<std::string, ScriptTypeId> type_names_;
  std::unordered_map<std::type_index, ScriptTypeId> type_indices_;
};

ScriptContext* global_script_context();

class ScriptEnvironment {
 public:
 private:
  ScriptEnvironment* parent = nullptr;
  std::map<std::string, ScriptVariable> variables_;
};

class ScriptChunk {
 public:
  enum class InstructionType : uint8_t {
    FUNCTION_CALL,
    PUSH_CONSTANT,
    PUSH_STACK_VARIABLE,
    JUMP_IF_TRUE,
    JUMP_IF_FALSE,
  };
  struct FunctionCall {
    uint8_t input_count;
    uint8_t output_count;
    ScriptFunctionPointer function;
  };
  struct PushConstant {
    ScriptVariable value;
  };
  struct PushStackValue {
    int position;
  };
  struct ConditionalJump {
    int instruction_offset;
  };
  struct Instruction {
    InstructionType type;
    std::variant<FunctionCall, PushConstant, PushStackValue, ConditionalJump> data;
  };

  ScriptChunk(const json& definition);

  ScriptFunctionResult Execute(std::span<const ScriptVariable> input);
  template <typename... Inputs>
  ScriptFunctionResult Execute(Inputs&&... inputs) {
    context_->PushValues(std::forward<Inputs>(inputs)...);
    return Execute();
  }
  void Print();

 private:
  ScriptContext* context_ = global_script_context();
  std::vector<Instruction> instructions_;
  std::vector<ScriptVariableDefinition> inputs_;
  std::vector<ScriptVariableDefinition> outputs_;

  ScriptFunctionResult Execute();
};

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType(ArgumentTypes...)> {
  using FunctionType = ReturnType (*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes);
  static constexpr size_t OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;

  template <FunctionType FUNCTION>
  static void Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ArgumentTypes...>(-input_count);
    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, inputs);
    } else {
      context->AssignValue(-input_count - output_count, std::apply(FUNCTION, inputs));
    }
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, INPUT_COUNT> input_type_ids;
    FillInputTypeIdArray(context, input_type_ids.data());
    return input_type_ids;
  }

  template <size_t N = 0>
  static void FillInputTypeIdArray(ScriptContext* context, ScriptTypeId* id) {
    if constexpr (N == sizeof...(ArgumentTypes)) {
      return;
    } else {
      *id = context->GetTypeId<typename std::tuple_element<N, std::tuple<ArgumentTypes...>>::type>();
      FillInputTypeIdArray<N + 1>(context, id + 1);
    }
  }

  static std::array<ScriptTypeId, OUTPUT_COUNT> GetOutputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, OUTPUT_COUNT> output_types_ids;
    if constexpr (OUTPUT_COUNT == 1) {
      output_types_ids[0] = context->GetTypeId<ReturnType>();
    }
    return output_types_ids;
  }
};

template <typename ReturnType, typename ObjectType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType (ObjectType::*)(ArgumentTypes...)> {
  using FunctionType = ReturnType (ObjectType::*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes) + 1;
  static constexpr size_t OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;

  template <FunctionType FUNCTION>
  static void Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ObjectType*, ArgumentTypes...>(-input_count);
    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, inputs);
    } else {
      context->AssignValue(-input_count - output_count, std::apply(FUNCTION, inputs));
    }
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, INPUT_COUNT> input_type_ids;
    FillInputTypeIdArray(context, input_type_ids.data());
    return input_type_ids;
  }

  template <size_t N = 0>
  static void FillInputTypeIdArray(ScriptContext* context, ScriptTypeId* id) {
    if constexpr (N == sizeof...(ArgumentTypes) + 1) {
      return;
    } else {
      *id = context->GetTypeId<typename std::tuple_element<N, std::tuple<ObjectType*, ArgumentTypes...>>::type>();
      FillInputTypeIdArray<N + 1>(context, id + 1);
    }
  }

  static std::array<ScriptTypeId, OUTPUT_COUNT> GetOutputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, OUTPUT_COUNT> output_types_ids;
    if constexpr (OUTPUT_COUNT == 1) {
      output_types_ids[0] = context->GetType<ReturnType>().id;
    }
    return output_types_ids;
  }
};

template <typename FunctionType, FunctionType FUNCTION>
void ScriptContext::RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names,
                                     std::vector<std::string> outputs_names) {
  using Wrapper = FunctionWrapper<FunctionType>;

  std::vector<ScriptVariableDefinition> inputs;
  inputs.reserve(Wrapper::INPUT_COUNT);

  const auto input_type_ids = Wrapper::GetInputTypeIds(this);
  for (size_t i = 0; i < Wrapper::INPUT_COUNT; ++i) {
    inputs.push_back({input_type_ids[i], i < inputs_names.size() ? inputs_names[i] : std::to_string(i)});
  }

  std::vector<ScriptVariableDefinition> outputs;
  inputs.reserve(Wrapper::OUTPUT_COUNT);

  const auto output_type_ids = Wrapper::GetOutputTypeIds(this);
  for (size_t i = 0; i < Wrapper::OUTPUT_COUNT; ++i) {
    outputs.push_back({output_type_ids[i], i < outputs_names.size() ? outputs_names[i] : std::to_string(i)});
  }

  RegisterFunction(identifier, &Wrapper::template Execute<FUNCTION>, inputs, outputs);
}

struct ScriptReference {
  std::string node_id;
  std::string output;

  static std::optional<ScriptReference> Parse(std::string_view reference);
};

}  // namespace ovis
