#pragma once

#include <string_view>

#include <ovis/utils/safe_pointer.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_type.hpp>
#include <ovis/core/script_chunk.hpp>

namespace ovis {

class ScriptContext {
 public:
  ScriptContext();

  template <typename T> inline ScriptType* RegisterType(std::string name);
  template <SafelyReferenceableObject T, SafelyReferenceableObject BaseType>
  inline ScriptType* RegisterType(std::string name);

  // With GetTypeId() you can get the type id (an integer) for a specific type either by giving its name
  // (GetTypeId("Number")) or its C++ type (GetType<int>()).
  inline ScriptTypeId GetTypeId(std::string_view name) const;
  template <Number T> inline ScriptTypeId GetTypeId() const;
  template <SafelyReferenceableObjectPointer T> inline ScriptTypeId GetTypeId() const;
  template <typename T> inline ScriptTypeId GetTypeId() const;
  template <> ScriptTypeId GetTypeId<std::string_view>() const { return GetTypeId("String"); }

  // GetType() lets you inspect all data related to a type.
  // Usage:
  // * with type name as string: GetType("Number")
  // * with type id: GetType(SCRIPT_TYPE_UNKNOWN)
  // * with C++ type: GetType<int>()
  const ScriptType* GetType(ScriptTypeId type_id) const { return &types_[type_id.value]; }
  const ScriptType* GetType(std::string_view name) { return GetType(GetTypeId(name)); }
  template <typename T> const ScriptType* GetType() const { return GetType(GetTypeId<T>()); }

  auto type_names() const { return Keys(type_names_); }

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
  std::variant<ScriptError, std::vector<ScriptValue>> Call(std::string_view function_identifier, Arguments&&... arguments);

  std::optional<ScriptError> LoadAsset(std::string_view asset_id);
  bool LoadDocumentation(std::string_view language = "en-US");
  void PrintDebugInfo();

  // Types
  template <Number T> inline ScriptValue CreateScriptValue(T&& number);
  template <SafelyReferenceableObject T> inline ScriptValue CreateScriptValue(T* value);
  template <SafelyReferenceableObject T> inline ScriptValue CreateScriptValue(T& value);
  template <typename T> inline ScriptValue CreateScriptValue(T&& value);

  ScriptValue& GetValue(int position, int frame = 0);
  template <Number T> inline std::variant<T, ScriptError> GetValue(int position, int frame = 0);
  // template <SafelyReferenceableObject T> inline std::variant<T, ScriptError> GetValue(int position, int frame = 0);
  template <SafelyReferenceableObjectPointer T> inline std::variant<T, ScriptError> GetValue(int position, int frame = 0);
  template <typename T> std::variant<T, ScriptError> GetValue(int position, int frame = 0);
  template <> std::variant<std::string_view, ScriptError> GetValue<std::string_view>(int position, int frame);

  // Stack manipulation
  inline std::span<ScriptValue> PushStack(size_t count);
  void PopStack(size_t count);

  void AssignValue(int offset, ScriptValue value, int frame = 0) { GetValue(offset, frame) = value;   }
  template <typename T> void AssignValue(int offset, T&& value, int frame = 0);

  void PushUnknown() { PushValue(ScriptValue{.type = SCRIPT_TYPE_UNKNOWN}); }
  void PushValue(ScriptValue value);
  template <typename T> void PushValue(T&& value) { PushValue(CreateScriptValue(value)); }

  std::span<ScriptValue> GetRange(int begin, int end);

  inline void PushStackFrame();
  inline void PopStackFrame();

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

template <typename T>
inline ScriptType* ScriptContext::RegisterType(std::string name) {
  ScriptTypeId id { types_.size() };
  SDL_assert(type_names_.find(name) == type_names_.end());
  SDL_assert(type_indices_.find(typeid(T)) == type_indices_.end());

  types_.emplace_back(id, name);
  type_names_.insert(std::make_pair(name, id));
  type_indices_.insert(std::make_pair(std::type_index(typeid(T)), id));

  return &types_.back();
}

template <SafelyReferenceableObject T, SafelyReferenceableObject BaseType>
inline ScriptType* ScriptContext::RegisterType(std::string name) {
  ScriptTypeId id { types_.size() };
  SDL_assert(type_names_.find(name) == type_names_.end());
  SDL_assert(type_indices_.find(typeid(T)) == type_indices_.end());
  SDL_assert(type_indices_.find(typeid(BaseType)) != type_indices_.end());

  types_.emplace_back(id, name, GetTypeId<BaseType>(), &ConvertBaseToDerived<BaseType, T>, &ConvertDerivedToBase<BaseType, T>);
  type_names_.insert(std::make_pair(name, id));
  type_indices_.insert(std::make_pair(std::type_index(typeid(T)), id));

  return &types_.back();
}

inline ScriptTypeId ScriptContext::GetTypeId(std::string_view name) const {
  const auto it = type_names_.find(std::string(name));
  if (it == type_names_.end()) {
    return SCRIPT_TYPE_UNKNOWN;
  } else {
    return it->second;
  }
}

template <Number T>
inline ScriptTypeId ScriptContext::GetTypeId() const {
  const auto it = type_indices_.find(typeid(double));
  if (it == type_indices_.end()) {
    return SCRIPT_TYPE_UNKNOWN;
  } else {
    return it->second;
  }
}

template <SafelyReferenceableObjectPointer T>
inline ScriptTypeId ScriptContext::GetTypeId() const {
  return GetTypeId<std::remove_pointer_t<T>>();
}

template <typename T>
inline ScriptTypeId ScriptContext::GetTypeId() const {
  const auto it = type_indices_.find(typeid(T));
  if (it == type_indices_.end()) {
    return SCRIPT_TYPE_UNKNOWN;
  } else {
    return it->second;
  }
}

std::span<ScriptValue> ScriptContext::PushStack(size_t count) {
  if (stack_.size() + count > stack_.capacity()) {
    throw std::runtime_error("Script stack overflow");
  }
  const size_t begin = stack_.size();
  stack_.resize(begin + count);

  return {stack_.data() + begin, count};
}

template <Number T>
ScriptValue ScriptContext::CreateScriptValue(T&& number) {
  return ScriptValue{.type = GetTypeId<T>(), .value = static_cast<double>(number)};
}

template <SafelyReferenceableObject T>
ScriptValue ScriptContext::CreateScriptValue(T* value) {
  return ScriptValue{.type = GetTypeId<T>(), .value = safe_ptr<T>(value)};
}

template <SafelyReferenceableObject T>
ScriptValue ScriptContext::CreateScriptValue(T& value) {
  return ScriptValue{.type = GetTypeId<T>(), .value = safe_ptr<T>(&value)};
}

template <typename T>
ScriptValue ScriptContext::CreateScriptValue(T&& value) {
  return ScriptValue{.type = GetTypeId<T>(), .value = value};
}

template <typename T>
void ScriptContext::AssignValue(int offset, T&& value, int frame) {
  AssignValue(offset, CreateScriptValue(value), frame);
}

inline void ScriptContext::PushValue(ScriptValue value) {
  if (stack_.size() >= stack_.capacity()) {
    std::runtime_error("Stack overflow!");
  }
  stack_.emplace_back(std::move(value));
}

inline void ScriptContext::PopStack(size_t count) {
  if (count > stack_.size()) {
    throw std::runtime_error("Script stack underflow");
  }
  stack_.resize(stack_.size() - count);
}

inline ScriptValue& ScriptContext::GetValue(int position, int frame) {
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

// template <SafelyReferenceableObject T>
// inline std::variant<T, ScriptError> ScriptContext::GetValue(int position, int frame) {

// }

template <SafelyReferenceableObjectPointer T>
inline std::variant<T, ScriptError> ScriptContext::GetValue(int position, int frame) {
  const ScriptValue& value = GetValue(position, frame);
  const ScriptTypeId target_type_id = GetTypeId<T>();
  if (target_type_id == value.type) [[likely]] {
    using SafePointer = safe_ptr<std::remove_pointer_t<T>>;
    assert(value.value.type() == typeid(SafePointer));
    SafePointer safe_ptr_value = std::any_cast<SafePointer>(value.value);
    return safe_ptr_value.get();
  } else {
    return ScriptError{.action = ScriptActionReference(),
                       .message = fmt::format("Expected {}, got {}", GetType<T>()->name, GetType(value.type)->name)};
  }
}

template <typename T>
inline std::variant<T, ScriptError> ScriptContext::GetValue(int offset, int frame) {
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

template <>
inline std::variant<std::string_view, ScriptError> ScriptContext::GetValue<std::string_view>(int offset, int frame) {
  const ScriptValue& value = GetValue(offset, frame);
  const ScriptTypeId target_type_id = GetTypeId<std::string>();
  if (target_type_id == value.type) [[likely]] {
    assert(value.value.type() == typeid(std::string));
    const std::string& string_value = std::any_cast<const std::string&>(value.value);
    return std::string_view(string_value);
  } else {
    return ScriptError{.action = ScriptActionReference(),
                       .message = fmt::format("Expected {}, got {}", GetType<std::string>()->name, GetType(value.type)->name)};
  }
}

template <Number T>
inline std::variant<T, ScriptError> ScriptContext::GetValue(int offset, int frame) {
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

inline std::span<ScriptValue> ScriptContext::GetRange(int begin, int end) {
  SDL_assert(begin < 0);
  SDL_assert(end <= 0);
  SDL_assert(begin <= end);
  SDL_assert(-begin <= stack_.size());
  SDL_assert(end - begin >= 0);
  return {&stack_[stack_.size() + begin], static_cast<size_t>(end - begin)};
}

inline void ScriptContext::PushStackFrame() { stack_frames_.push_back(stack_.size()); }
inline void ScriptContext::PopStackFrame() {
  stack_.resize(stack_frames_.back());
  stack_frames_.pop_back();
}

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


}

namespace ovis {

inline ScriptChunkArguments::ScriptChunkArguments(const ScriptChunk& chunk)
    : chunk_(chunk), arguments_(chunk.inputs.size(), ScriptValue{ .type = SCRIPT_TYPE_UNKNOWN }) {
}

inline bool ScriptChunkArguments::Add(std::string_view identifier, ScriptValue value) {
  for (const auto& input : IndexRange(chunk_.inputs)) {
    if (input->identifier == identifier) {
      arguments_[input.index()] = std::move(value);
      return true;
    }
  }
  return false;
}

template <typename T>
inline bool ScriptChunkArguments::Add(std::string_view identifier, T&& value) {
  return Add(identifier, chunk_.context_->CreateScriptValue(value));
}

}
