#pragma once

#include <any>
#include <functional>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <typeindex>
#include <variant>

#include <SDL_assert.h>
#include <fmt/format.h>

#include <ovis/utils/range.hpp>
#include <ovis/utils/serialize.hpp>

namespace ovis {

class ScriptContext;

using ScriptTypeId = size_t;
constexpr ScriptTypeId SCRIPT_TYPE_UNKNOWN = 0;

struct ScriptType {
  ScriptType(ScriptTypeId id, std::string_view name, bool reference_type)
      : id(id), name(name), reference_type(reference_type) {}

  ScriptTypeId id;
  std::string name;
  bool reference_type;
};

struct ScriptValueDefinition {
  ScriptTypeId type;
  std::string identifier;
};

struct ScriptValue {
  ScriptTypeId type;
  std::any value;
};

struct ScriptActionReference {
  std::string string;

  static ScriptActionReference Root() { return {.string = "/"}; }
};

inline bool operator==(const ScriptActionReference& lhs, const ScriptActionReference& rhs) {
  return lhs.string == rhs.string;
}

inline ScriptActionReference operator/(const ScriptActionReference& reference, int i) {
  assert(reference.string != "");

  if (reference.string == "/") {
    return ScriptActionReference{.string = reference.string + std::to_string(i)};
  } else {
    return ScriptActionReference{.string = reference.string + "/" + std::to_string(i)};
  }
}


struct ScriptError {
  ScriptActionReference action;
  std::string message;
};

using ScriptFunctionParameters = std::vector<ScriptValue>;

using ScriptFunctionPointer = std::optional<ScriptError> (*)(ScriptContext* context, int input_count, int output_count);

struct ScriptFunction {
  ScriptFunctionPointer function;
  std::vector<ScriptValueDefinition> inputs;
  std::vector<ScriptValueDefinition> outputs;

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

struct ScriptFunctionResult {
  std::vector<ScriptValue> output_values;
  std::optional<ScriptError> error;
};

struct ScriptParseError {
  size_t action_id;
  std::string message;
};

template <typename T>
concept Number = (std::is_integral_v<std::remove_reference_t<std::remove_cv_t<T>>> &&
                  !std::is_same_v<std::remove_reference_t<std::remove_cv_t<T>>, bool>) ||
                 std::is_floating_point_v<std::remove_reference_t<std::remove_cv_t<T>>>;

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

  const ScriptType* GetType(ScriptTypeId type_id) const { return &types_[type_id]; }

  ScriptTypeId GetTypeId(std::string_view name) {
    const auto it = type_names_.find(std::string(name));
    if (it == type_names_.end()) {
      return SCRIPT_TYPE_UNKNOWN;
    } else {
      return it->second;
    }
  }

  const ScriptType* GetType(std::string_view name) {
    return GetType(GetTypeId(name));
  }
  template <Number T>
  ScriptTypeId GetTypeId() const {
    const auto it = type_indices_.find(typeid(double));
    if (it == type_indices_.end()) {
      return SCRIPT_TYPE_UNKNOWN;
    } else {
      return it->second;
    }
  }

  template <typename T>
  ScriptTypeId GetTypeId() const {
    const auto it = type_indices_.find(typeid(T));
    if (it == type_indices_.end()) {
      return SCRIPT_TYPE_UNKNOWN;
    } else {
      return it->second;
    }
  }

  template <typename T>
  const ScriptType* GetType() const {
    return GetType(GetTypeId<T>());
  }

  template <Number T>
  ScriptValue CreateScriptValue(T&& number) {
    return ScriptValue{.type = GetTypeId<T>(), .value = static_cast<double>(number)};
  }

  template <typename T>
  ScriptValue CreateScriptValue(T&& value) {
    return ScriptValue{.type = GetTypeId<T>(), .value = value};
  }

  void RegisterFunction(std::string_view identifier, ScriptFunctionPointer function,
                        std::span<const ScriptValueDefinition> inputs, std::span<const ScriptValueDefinition> outputs);

  template <auto FUNCTION>
  void RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names = {},
                        std::vector<std::string> outputs_names = {});

  template <typename T, typename... ConstructorArguments>
  void RegisterConstructor(std::string_view identifier, std::vector<std::string> input_names = {},
                           std::string_view output_name = "");

  const ScriptFunction* GetFunction(std::string_view identifier);

  std::variant<ScriptError, std::vector<ScriptValue>> Execute(std::string_view function_identifier,
                                                              std::span<ScriptValue> arguments);

  std::span<ScriptValue> PushStack(size_t count) {
    if (stack_.size() + count > stack_.capacity()) {
      throw std::runtime_error("Script stack overflow");
    }
    const size_t begin = stack_.size();
    stack_.resize(begin + count);

    return {stack_.data() + begin, count};
  }

  void AssignValue(int offset, ScriptValue value) { GetValue(offset) = value; }

  template <typename T>
  void AssignValue(int offset, T&& value) {
    AssignValue(offset, CreateScriptValue(value));
  }

  void PushValue(ScriptValue value) {
    if (stack_.size() >= stack_.capacity()) {
      std::runtime_error("Stack overflow!");
    }
    stack_.emplace_back(std::move(value));
  }

  template <typename T>
  void PushValue(T&& value) {
    PushValue(CreateScriptValue(value));
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

  ScriptValue& GetValue(int offset) {
    SDL_assert(offset < 0);
    return stack_[stack_.size() + offset];
  }

  template <Number T>
  std::variant<T, ScriptError> GetValue(int offset) {
    const ScriptValue& value = GetValue(offset);
    const ScriptTypeId target_type_id = GetTypeId<T>();
    if (target_type_id == value.type) [[likely]] {
      assert(value.value.type() == typeid(double));
      return static_cast<T>(std::any_cast<double>(value.value));
    } else {
      return ScriptError{.action = ScriptActionReference(),
                         .message = fmt::format("Expected {}, got {}", GetType<T>()->name, GetType(value.type)->name)};
    }
  }

  template <typename T>
  std::variant<T, ScriptError> GetValue(int offset) {
    const ScriptValue& value = GetValue(offset);
    const ScriptTypeId target_type_id = GetTypeId<T>();
    if (target_type_id == value.type) [[likely]] {
      assert(value.value.type() == typeid(T));
      return std::any_cast<T>(value.value);
    } else {
      return ScriptError{.action = ScriptActionReference(),
                         .message = fmt::format("Expected {}, got {}", GetType<T>()->name, GetType(value.type)->name)};
    }
  }

  template <typename... T>
  std::variant<std::tuple<std::remove_cv_t<std::remove_reference_t<T>>...>, ScriptError> GetValues(int begin) {
    std::tuple<std::remove_cv_t<std::remove_reference_t<T>>...> values;

    ScriptError error;
    if (FillValueTuple<0>(&values, begin, &error)) [[likely]] {
      return values;
    } else {
      return error;
    }
  }

  template <int N, typename... T>
  bool FillValueTuple(std::tuple<T...>* values, int begin, ScriptError* error) {
    SDL_assert(error != nullptr);

    if constexpr (N == sizeof...(T)) {
      return true;
    } else {
      using Type = std::tuple_element_t<N, std::tuple<T...>>;
      auto value = GetValue<Type>(begin + N);
      if (std::holds_alternative<Type>(value)) [[likely]] {
        std::get<N>(*values) = std::move(std::get<Type>(value));
        return FillValueTuple<N + 1>(values, begin, error);
      } else {
        const auto& inner_error = std::get<ScriptError>(value);
        error->action = inner_error.action;
        error->message = fmt::format("{}: {}", N + 1, inner_error.message);
        return false;
      }
    }
  }

  template <typename... T>
  std::array<ScriptTypeId, sizeof...(T)> GetTypeIds() const {
    std::array<ScriptTypeId, sizeof...(T)> input_type_ids;
    FillInputTypeIdArray<0, T...>(input_type_ids.data());
    return input_type_ids;
  }

  template <size_t N, typename... T>
  void FillInputTypeIdArray(ScriptTypeId* id) const {
    if constexpr (N == sizeof...(T)) {
      return;
    } else {
      *id = GetTypeId<typename std::tuple_element_t<N, std::tuple<T...>>>();
      FillInputTypeIdArray<N + 1, T...>(id + 1);
    }
  }

  // static std::array<ScriptTypeId, OUTPUT_COUNT> GetOutputTypeIds(ScriptContext* context) {
  //   std::array<ScriptTypeId, OUTPUT_COUNT> output_types_ids;
  //   if constexpr (OUTPUT_COUNT == 1) {
  //     output_types_ids[0] = context->GetTypeId<ReturnType>();
  //   }
  //   return output_types_ids;
  // }

  std::span<ScriptValue> GetRange(int begin, int end) {
    SDL_assert(begin < 0);
    SDL_assert(end <= 0);
    SDL_assert(begin <= end);
    SDL_assert(-begin <= stack_.size());
    SDL_assert(end - begin >= 0);
    return {&stack_[stack_.size() + begin], static_cast<size_t>(end - begin)};
  }

 private:
  std::map<std::string, ScriptFunction, std::less<>> functions_;
  std::vector<ScriptValue> stack_;
  std::vector<ScriptType> types_;
  std::unordered_map<std::string, ScriptTypeId> type_names_;
  std::unordered_map<std::type_index, ScriptTypeId> type_indices_;
};

ScriptContext* global_script_context();

class ScriptEnvironment {
 public:
 private:
  ScriptEnvironment* parent = nullptr;
  std::map<std::string, ScriptValue> variables_;
};

class ScriptChunk {
 public:
  enum class InstructionType : uint8_t {
    FUNCTION_CALL,
    PUSH_CONSTANT,
    PUSH_STACK_VARIABLE,
    ASSIGN_CONSTANT,
    ASSIGN_STACK_VARIABLE,
    POP,
    JUMP_IF_TRUE,
    JUMP_IF_FALSE,
  };
  struct FunctionCall {
    uint8_t input_count;
    uint8_t output_count;
    ScriptFunctionPointer function;
  };
  struct PushConstant {
    ScriptValue value;
  };
  struct PushStackValue {
    int position;
  };
  struct AssignConstant {
    ScriptValue value;
    int position;
  };
  struct AssignStackVariable {
    int source_position;
    int destination_position;
  };
  struct Pop {
    int count;
  };
  struct ConditionalJump {
    int instruction_offset;
  };
  struct Instruction {
    InstructionType type;
    std::variant<FunctionCall, PushConstant, PushStackValue, Pop, AssignConstant, AssignStackVariable, ConditionalJump> data;
  };


  static std::variant<ScriptChunk, ScriptError> Load(const json& definition, ScriptContext* context = global_script_context());

  ScriptFunctionResult Execute(std::span<const ScriptValue> input);
  template <typename... Inputs>
  ScriptFunctionResult Execute(Inputs&&... inputs) {
    context_->PushValues(std::forward<Inputs>(inputs)...);
    return Execute();
  }

  std::vector<ScriptValueDefinition> GetVisibleLocalVariables(ScriptActionReference action);

  void Print();

 private:
  ScriptChunk(ScriptContext* context);

  // Needed for running
  ScriptContext* context_ = global_script_context();
  std::vector<Instruction> instructions_;
  std::vector<ScriptValueDefinition> inputs_;
  std::vector<ScriptValueDefinition> outputs_;

  // Only necessary for parsing / debugging
  std::vector<ScriptActionReference> instruction_to_action_mappings_;
  struct LocalVariable {
    std::string name;
    ScriptActionReference declaring_action;
    ScriptTypeId type;
    int position;
  };
  std::vector<LocalVariable> local_variables_;

  struct Scope {
    std::vector<ScriptChunk::Instruction> instructions;
    std::vector<ScriptActionReference> instruction_to_action_mappings;
  };
  std::variant<Scope, ScriptError> ParseScope(const json& actions, const ScriptActionReference& parent = ScriptActionReference::Root());

  std::optional<LocalVariable> GetLocalVariable(std::string_view name);
  ScriptFunctionResult Execute();
};

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType(*)(ArgumentTypes...)> {
  using FunctionType = ReturnType (*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes);
  static constexpr size_t OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;

  template <FunctionType FUNCTION>
  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ArgumentTypes...>(-input_count);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }

    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, std::get<0>(inputs));
    } else {
      context->AssignValue(-input_count - output_count, std::apply(FUNCTION, std::get<0>(inputs)));
    }

    return {};
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    return context->GetTypeIds<ArgumentTypes...>();
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
  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ObjectType*, ArgumentTypes...>(-input_count);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }

    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, std::get<0>(inputs));
    } else {
      context->AssignValue(-input_count - output_count, std::apply(FUNCTION, std::get<0>(inputs)));
    }

    return {};
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    return context->GetTypeIds<ObjectType*, ArgumentTypes...>();
  }

  static std::array<ScriptTypeId, OUTPUT_COUNT> GetOutputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, OUTPUT_COUNT> output_types_ids;
    if constexpr (OUTPUT_COUNT == 1) {
      output_types_ids[0] = context->GetTypeId<ReturnType>();
    }
    return output_types_ids;
  }
};

template <typename T, typename... ConstructorArguments>
struct ConstructorWrapper {
  static constexpr auto INPUT_COUNT = sizeof...(ConstructorArguments);
  static constexpr size_t OUTPUT_COUNT = 1;

  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ConstructorArguments...>(-input_count);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }
    context->AssignValue(-input_count - output_count, std::make_from_tuple<T>(std::move(std::get<0>(inputs))));
    return {};
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    return context->GetTypeIds<ConstructorArguments...>();
  }

  static std::array<ScriptTypeId, OUTPUT_COUNT> GetOutputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, OUTPUT_COUNT> output_types_ids;
    output_types_ids[0] = context->GetTypeId<T>();
    return output_types_ids;
  }
};

template <auto FUNCTION>
void ScriptContext::RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names,
                                     std::vector<std::string> outputs_names) {
  using FunctionType = decltype(FUNCTION);
  using Wrapper = FunctionWrapper<FunctionType>;

  std::vector<ScriptValueDefinition> inputs;
  inputs.reserve(Wrapper::INPUT_COUNT);

  const auto input_type_ids = Wrapper::GetInputTypeIds(this);
  for (size_t i = 0; i < Wrapper::INPUT_COUNT; ++i) {
    inputs.push_back({input_type_ids[i], i < inputs_names.size() ? inputs_names[i] : std::to_string(i)});
  }

  std::vector<ScriptValueDefinition> outputs;
  outputs.reserve(Wrapper::OUTPUT_COUNT);

  const auto output_type_ids = Wrapper::GetOutputTypeIds(this);
  for (size_t i = 0; i < Wrapper::OUTPUT_COUNT; ++i) {
    outputs.push_back({output_type_ids[i], i < outputs_names.size() ? outputs_names[i] : std::to_string(i)});
  }

  RegisterFunction(identifier, &Wrapper::template Execute<FUNCTION>, inputs, outputs);
}

template <typename T, typename... ConstructorArguments>
void ScriptContext::RegisterConstructor(std::string_view identifier, std::vector<std::string> input_names,
                                        std::string_view output_name) {
  using Wrapper = ConstructorWrapper<T, ConstructorArguments...>;

  std::vector<ScriptValueDefinition> inputs;
  inputs.reserve(Wrapper::INPUT_COUNT);

  const auto input_type_ids = Wrapper::GetInputTypeIds(this);
  for (size_t i = 0; i < Wrapper::INPUT_COUNT; ++i) {
    inputs.push_back({input_type_ids[i], i < input_names.size() ? input_names[i] : std::to_string(i)});
  }

  std::vector<ScriptValueDefinition> outputs;
  const auto output_type_ids = Wrapper::GetOutputTypeIds(this);
  outputs.push_back({ output_type_ids[0], std::string(output_name.length() > 0 ? output_name : "0")});

  RegisterFunction(identifier, &Wrapper::Execute, inputs, outputs);
}

struct ScriptReference {
  std::string node_id;
  std::string output;

  static std::optional<ScriptReference> Parse(std::string_view reference);
};

}  // namespace ovis
