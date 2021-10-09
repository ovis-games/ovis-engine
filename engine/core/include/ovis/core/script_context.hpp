#pragma once

#include <string_view>

#include <ovis/core/script_function.hpp>
#include <ovis/core/script_type.hpp>
#include <ovis/core/script_chunk.hpp>

namespace ovis {

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

  const ScriptType* GetType(std::string_view name) { return GetType(GetTypeId(name)); }

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

  std::optional<ScriptError> RegisterFunction(std::string_view function_id, json definition);

  void RegisterFunction(std::string_view identifier, ScriptChunk script_chunk);

  void RegisterFunction(std::string_view identifier, ScriptFunctionPointer function,
                        std::span<const ScriptValueDefinition> inputs, std::span<const ScriptValueDefinition> outputs);

  template <auto FUNCTION>
  void RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names = {},
                        std::vector<std::string> outputs_names = {});

  template <typename T, typename... ConstructorArguments>
  void RegisterConstructor(std::string_view identifier, std::vector<std::string> input_names = {},
                           std::string_view output_name = "");

  const ScriptFunction* GetFunction(std::string_view identifier);
  auto function_identifiers() const { return Keys(functions_); }

  std::variant<ScriptError, std::vector<ScriptValue>> Call(std::string_view function_identifier,
                                                              std::span<ScriptValue> arguments);
  template <typename... Arguments>
  std::variant<ScriptError, std::vector<ScriptValue>> Call(std::string_view function_identifier, Arguments&&... arguments) {
    if (stack_.size() != 0) {
      return ScriptError{
        .message = fmt::format("Cannot execute function while other execution is in progress")
      };
    }

    const auto function = GetFunction(function_identifier);
    if (function == nullptr) {
      return ScriptError{
        .message = fmt::format("Unknown function: {}", function_identifier)
      };
    }

    for (const auto& output : function->outputs) {
      PushUnknown();
    }
    PushValues(std::forward<Arguments>(arguments)...);

    if (const auto native_function = dynamic_cast<const NativeScriptFunction*>(function); native_function != nullptr) {
      const auto error = native_function->function(this, sizeof...(Arguments), function->outputs.size());
      if (error.has_value()) {
        return *error;
      }
      std::vector<ScriptValue> values;
      for (const auto& output : function->outputs) {
        values.push_back(std::move(GetValue(values.size())));
      }
      stack_.clear();
      return values;
    } else {
      return ScriptError{
        .message = "Currently only native functions are executable"
      };
    }
  }


  std::span<ScriptValue> PushStack(size_t count) {
    if (stack_.size() + count > stack_.capacity()) {
      throw std::runtime_error("Script stack overflow");
    }
    const size_t begin = stack_.size();
    stack_.resize(begin + count);

    return {stack_.data() + begin, count};
  }

  void AssignValue(int offset, ScriptValue value, int frame = 0) { GetValue(offset, frame) = value; }

  template <typename T>
  void AssignValue(int offset, T&& value, int frame = 0) {
    AssignValue(offset, CreateScriptValue(value), frame);
  }

  void PushUnknown() { PushValue(ScriptValue{.type = SCRIPT_TYPE_UNKNOWN}); }

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

  void PushValues() {}

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

  ScriptValue& GetValue(int position, int frame = 0) {
    SDL_assert(frame <= 0);
    SDL_assert(stack_frames_.size() > 0);

    if (position >= 0) {
      const int frame_begin = stack_frames_[stack_frames_.size() + frame - 1];
      return stack_[frame_begin + position];
    } else {
      const int frame_end = frame == 0 ? stack_.size() : stack_frames_[stack_frames_.size() + frame + 1];
      return stack_[frame_end + position];
    }
  }

  template <Number T>
  std::variant<T, ScriptError> GetValue(int offset, int frame = 0) {
    const ScriptValue& value = GetValue(offset, frame);
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
  std::variant<T, ScriptError> GetValue(int offset, int frame = 0) {
    const ScriptValue& value = GetValue(offset, frame);
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
  std::variant<std::tuple<std::remove_cv_t<std::remove_reference_t<T>>...>, ScriptError> GetValues(int begin, int frame = 0) {
    std::tuple<std::remove_cv_t<std::remove_reference_t<T>>...> values;

    ScriptError error;
    if (FillValueTuple<0>(&values, begin, frame, &error)) [[likely]] {
      return values;
    } else {
      return error;
    }
  }

  template <int N, typename... T>
  bool FillValueTuple(std::tuple<T...>* values, int begin, int frame, ScriptError* error) {
    SDL_assert(error != nullptr);

    if constexpr (N == sizeof...(T)) {
      return true;
    } else {
      using Type = std::tuple_element_t<N, std::tuple<T...>>;
      auto value = GetValue<Type>(begin + N, frame);
      if (std::holds_alternative<Type>(value)) [[likely]] {
        std::get<N>(*values) = std::move(std::get<Type>(value));
        return FillValueTuple<N + 1>(values, begin, frame, error);
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

  std::span<ScriptValue> GetRange(int begin, int end) {
    SDL_assert(begin < 0);
    SDL_assert(end <= 0);
    SDL_assert(begin <= end);
    SDL_assert(-begin <= stack_.size());
    SDL_assert(end - begin >= 0);
    return {&stack_[stack_.size() + begin], static_cast<size_t>(end - begin)};
  }

  std::optional<ScriptError> LoadAsset(std::string_view asset_id);
  bool LoadDocumentation(std::string_view language = "en-US");
  void PrintDebugInfo();

  void PushStackFrame() { stack_frames_.push_back(stack_.size()); }
  void PopStackFrame() {
    stack_.resize(stack_frames_.back());
    stack_frames_.pop_back();
  }

 private:
  std::map<std::string, std::unique_ptr<ScriptFunction>, std::less<>> functions_;

  std::vector<ScriptValue> stack_;
  std::vector<int> stack_frames_;

  std::vector<ScriptType> types_;
  std::unordered_map<std::string, ScriptTypeId> type_names_;
  std::unordered_map<std::type_index, ScriptTypeId> type_indices_;
};

ScriptContext* global_script_context();
}

#include <ovis/core/script_wrappers.hpp>

namespace ovis {

template <auto FUNCTION>
void ScriptContext::RegisterFunction(std::string_view identifier, std::vector<std::string> inputs_names,
                                     std::vector<std::string> outputs_names) {
  using FunctionType = decltype(FUNCTION);
  using Wrapper = ScriptFunctionWrapper<FunctionType>;

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
  using Wrapper = ScriptConstructorWrapper<T, ConstructorArguments...>;

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

template <typename... Inputs>
std::variant<ScriptError, std::vector<ScriptValue>> ScriptChunk::Call(Inputs&&... inputs) {
  std::array<ScriptValue, sizeof...(Inputs)> arguments = { context_->CreateScriptValue(inputs)... };
  return Call(std::span<const ScriptValue>(arguments));
}

}

