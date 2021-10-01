#pragma once

namespace ovis {

template <typename FunctionType>
struct ScriptFunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct ScriptFunctionWrapper<ReturnType(*)(ArgumentTypes...)> {
  using FunctionType = ReturnType (*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes);
  static constexpr size_t OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;

  template <FunctionType FUNCTION>
  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ArgumentTypes...>(0);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }

    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, std::get<0>(inputs));
    } else {
      context->PushValue(std::apply(FUNCTION, std::get<0>(inputs)));
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
struct ScriptFunctionWrapper<ReturnType (ObjectType::*)(ArgumentTypes...)> {
  using FunctionType = ReturnType (ObjectType::*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes) + 1;
  static constexpr size_t OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;

  template <FunctionType FUNCTION>
  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ObjectType*, ArgumentTypes...>(0);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }

    if constexpr (OUTPUT_COUNT == 0) {
      std::apply(FUNCTION, std::get<0>(inputs));
    } else {
      context->PushValue(std::apply(FUNCTION, std::get<0>(inputs)));
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
struct ScriptConstructorWrapper {
  static constexpr auto INPUT_COUNT = sizeof...(ConstructorArguments);
  static constexpr size_t OUTPUT_COUNT = 1;

  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    const auto inputs = context->GetValues<ConstructorArguments...>(0);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }
    context->PushValue(std::make_from_tuple<T>(std::move(std::get<0>(inputs))));
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

}
