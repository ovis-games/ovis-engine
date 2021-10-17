#pragma once

#include <ovis/utils/safe_pointer.hpp>
#include <ovis/core/script_type.hpp>

namespace ovis {

template <SafelyReferenceableObject BaseType, SafelyReferenceableObject DerivedType>
ScriptValue ConvertBaseToDerived(ScriptContext* context, const ScriptValue& base_value) {
  assert(context->GetTypeId<BaseType>() == base_value.type);
  assert(context->GetType<DerivedType>()->base_type_id == context->GetTypeId<BaseType>());
  assert(base_value.value.type() == typeid(BaseType*));
  BaseType* base = std::get<BaseType*>(base_value.value);
  return ScriptValue {
    .value = down_cast<DerivedType*>(base),
    .type = context->GetTypeId<DerivedType>(),
  };
}

template <SafelyReferenceableObject BaseType, SafelyReferenceableObject DerivedType>
ScriptValue ConvertDerivedToBase(ScriptContext* context, const ScriptValue& derived_value) {
  assert(context->GetTypeId<DerivedType>() == derived_value.type);
  assert(context->GetType<DerivedType>()->base_type_id == context->GetTypeId<BaseType>());
  assert(derived_value.value.type() == typeid(DerivedType*));
  BaseType* base = std::get<DerivedType*>(derived_value.value);
  return ScriptValue {
    .value = base,
    .type = context->GetTypeId<BaseType>(),
  };
}

template <typename FunctionType>
struct ScriptFunctionWrapper;

namespace detail {

template <int N, typename... T>
bool FillValueTuple(std::tuple<T...>* values, ScriptContext* context, ScriptError* error) {
  SDL_assert(error != nullptr);

  if constexpr (N == sizeof...(T)) {
    return true;
  } else {
    using Type = std::tuple_element_t<N, std::tuple<T...>>;
    auto value = context->GetValue<Type>(N);
    if (std::holds_alternative<Type>(value)) [[likely]] {
      std::get<N>(*values) = std::move(std::get<Type>(value));
      return FillValueTuple<N + 1>(values, context, error);
    } else {
      const auto& inner_error = std::get<ScriptError>(value);
      error->action = inner_error.action;
      error->message = fmt::format("{}: {}", N + 1, inner_error.message);
      return false;
    }
  }
}

template <typename... T>
std::variant<std::tuple<std::remove_cv_t<std::remove_reference_t<T>>...>, ScriptError> ConstructArgumentTuple(
    ScriptContext* context) {
  std::tuple<std::remove_cv_t<std::remove_reference_t<T>>...> values;

  ScriptError error;
  if (FillValueTuple<0>(&values, context, &error)) [[likely]] {
    return values;
  } else {
    return error;
  }
}

template <typename ResultType, typename... ArgumentTypes>
struct FunctionResult {
  static std::optional<ScriptError> Push(ScriptContext* context, ResultType&& result,
                                         const std::tuple<ArgumentTypes...>& inputs) {
    context->PushValue(result);
    return {};
  }
};

template <size_t DEPENDANT_INPUT_INDEX, typename ResultBaseType, typename... ArgumentTypes>
struct FunctionResult<InputDependentScriptValue<DEPENDANT_INPUT_INDEX, ResultBaseType>, ArgumentTypes...> {
  static std::optional<ScriptError> Push(ScriptContext* context,
                                         InputDependentScriptValue<DEPENDANT_INPUT_INDEX, ResultBaseType>&& result,
                                         const std::tuple<ArgumentTypes...>& inputs) {
    // TODO: cast value properly
    context->PushValue(result.value);
    return {};
  }
};

template <auto FUNCTION, typename ResultType, typename... ArgumentTypes>
std::optional<ScriptError> CallFunction(ScriptContext* context) {
  const auto inputs = detail::ConstructArgumentTuple<ArgumentTypes...>(context);
  if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
    ScriptError error = std::get<ScriptError>(inputs);
    error.message = fmt::format("Parameter {}", error.message);
    return error;
  }

  if constexpr (std::is_same_v<ResultType, void>) {
    std::apply(FUNCTION, std::get<0>(inputs));
  } else {
    const auto input_tuple = std::get<0>(inputs);
    FunctionResult<ResultType, ArgumentTypes...>::Push(context, std::apply(FUNCTION, input_tuple), input_tuple);
  }
  return {};
}
}

template <typename ReturnType, typename... ArgumentTypes>
struct ScriptFunctionWrapper<ReturnType(*)(ArgumentTypes...)> {
  using FunctionType = ReturnType (*)(ArgumentTypes...);
  static constexpr auto INPUT_COUNT = sizeof...(ArgumentTypes);
  static constexpr size_t OUTPUT_COUNT = std::is_same_v<ReturnType, void> ? 0 : 1;

  template <FunctionType FUNCTION>
  static std::optional<ScriptError> Execute(ScriptContext* context, int input_count, int output_count) {
    return detail::CallFunction<FUNCTION, ReturnType, ArgumentTypes...>(context);
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    return { context->GetTypeId<ArgumentTypes>()... };
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
    return detail::CallFunction<FUNCTION, ReturnType, ObjectType*, ArgumentTypes...>(context);
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    return { context->GetTypeId<ObjectType*>(), context->GetTypeId<ArgumentTypes>()... };
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
    const auto inputs = detail::ConstructArgumentTuple<ConstructorArguments...>(context);
    if (std::holds_alternative<ScriptError>(inputs)) [[unlikely]] {
      ScriptError error = std::get<ScriptError>(inputs);
      error.message = fmt::format("Parameter {}", error.message);
      return error;
    }
    context->PushValue(std::make_from_tuple<T>(std::move(std::get<0>(inputs))));
    return {};
  }

  static std::array<ScriptTypeId, INPUT_COUNT> GetInputTypeIds(ScriptContext* context) {
    return { context->GetTypeId<ConstructorArguments>()... };
  }

  static std::array<ScriptTypeId, OUTPUT_COUNT> GetOutputTypeIds(ScriptContext* context) {
    std::array<ScriptTypeId, OUTPUT_COUNT> output_types_ids;
    output_types_ids[0] = context->GetTypeId<T>();
    return output_types_ids;
  }
};

}
