namespace ovis {
namespace vm {

inline safe_ptr<Module> Module::Register(std::string_view name) {
  if (Get(name) != nullptr) {
    return nullptr;
  }

  modules.push_back(Module(name));
  return safe_ptr(&modules.back());
}

inline void Module::Deregister(std::string_view name) {
  std::erase_if(modules, [name](const Module& module) {
    return module.name_ == name;
  });
}

inline safe_ptr<Module> Module::Get(std::string_view name) {
  for (auto& module : modules) {
    if (module.name_ == name) {
      return safe_ptr(&module);
    }
  }
  return nullptr;
}

inline safe_ptr<Type> Module::RegisterType(std::string_view name) {
  if (GetType(name) != nullptr) {
    return nullptr;
  }

  types_.push_back(Type(this, name));
  return safe_ptr(&types_.back());
}

inline safe_ptr<Type> Module::RegisterType(std::string_view name, safe_ptr<Type> parent_type, Type::ConversionFunction to_base,
                              Type::ConversionFunction from_base) {
  if (GetType(name) != nullptr) {
    return nullptr;
  }

  types_.push_back(Type(this, name, parent_type.get(), to_base, from_base));
  return safe_ptr(&types_.back());
}

namespace detail {

template <typename Base, typename Derived>
Value FromBase(Value& base) {
  assert(base.is_view()); // If the value is not a view it cannot be something else
  return Value::CreateView(down_cast<Derived&>(base.Get<Base>()));
}

template <typename Base, typename Derived>
Value ToBase(Value& derived) {
  return Value::CreateView(static_cast<Base&>(derived.Get<Derived>()));
}

}

template <typename T, typename ParentType>
inline safe_ptr<Type> Module::RegisterType(std::string_view name, bool create_cpp_association) {
  if (Type::Get<T>() != nullptr) {
    return nullptr;
  }

  if (create_cpp_association && Type::type_associations[typeid(T)] != nullptr) {
    return nullptr;
  }

  safe_ptr<Type> type;
  if constexpr (std::is_same_v<ParentType, void>) {
    type = RegisterType(name);
  } else {
    type = RegisterType(name, Type::Get<ParentType>(), &detail::ToBase<ParentType, T>, &detail::FromBase<ParentType, T>);
  }
  if (type == nullptr) {
    return nullptr;
  }

  if (create_cpp_association) {
    Type::type_associations[typeid(T)] = type;
  }

  return type;
}

inline safe_ptr<Type> Module::GetType(std::string_view name) {
  for (auto& type : types_) {
    if (type.name() == name) {
      return safe_ptr(&type);
    }
  }
  return nullptr;
}

inline safe_ptr<Function> Module::RegisterFunction(std::string_view name, FunctionPointer function_pointer,
                                                   std::vector<Function::ValueDeclaration> inputs,
                                                   std::vector<Function::ValueDeclaration> outputs) {
  if (GetFunction(name) != nullptr) {
    return nullptr;
  }
  functions_.push_back(Function(name, function_pointer, std::forward<std::vector<Function::ValueDeclaration>>(inputs),
                                std::forward<std::vector<Function::ValueDeclaration>>(outputs)));
  return safe_ptr(&functions_.back());
}
namespace detail {

template <typename T>
struct VariableDeclarationsHelper {
  static void Insert(std::vector<Function::ValueDeclaration>* declarations, std::span<std::string_view> variable_names) {
    assert(declarations->size() < variable_names.size());
    declarations->push_back({
        std::string(variable_names[declarations->size()]),
        Type::Get<T>()
    });
  }
};
template <>
struct VariableDeclarationsHelper<void> {
  static void Insert(std::vector<Function::ValueDeclaration>* declarations, std::span<std::string_view> variable_names) {
  }
};
template <typename... T>
struct VariableDeclarationsHelper<std::tuple<T...>> {
  static void Insert(std::vector<Function::ValueDeclaration>* declarations, std::span<std::string_view> variable_names) {
    ((VariableDeclarationsHelper<T>::Insert(declarations, variable_names)), ...);
  }
};

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType (*)(ArgumentTypes...)> {
  using FunctionPointerType = ReturnType (*)(ArgumentTypes...);
  using ArgumentTuple = std::tuple<ArgumentTypes...>;

  static std::vector<Function::ValueDeclaration> GetInputDeclarations(std::span<std::string_view> input_names) {
    std::vector<Function::ValueDeclaration> input_declarations;
    VariableDeclarationsHelper<ArgumentTuple>::Insert(&input_declarations, input_names);
    assert(input_declarations.size() == input_names.size());
    return input_declarations;
  }

  static std::vector<Function::ValueDeclaration> GetOutputDeclarations(std::span<std::string_view> output_names) {
    std::vector<Function::ValueDeclaration> output_declarations;
    VariableDeclarationsHelper<ReturnType>::Insert(&output_declarations, output_names);
    assert(output_declarations.size() == output_names.size());
    return output_declarations;
  }

  template <FunctionPointerType FUNCTION>
  static void Call(ExecutionContext* context) {
    if constexpr (std::is_same_v<ReturnType, void>) {
      std::apply(FUNCTION, context->GetValue<ArgumentTuple>(0));
    } else {
      context->PushValue(std::apply(FUNCTION, context->GetValue<ArgumentTuple>(0)));
    }
  }
};

template <typename T, typename... ConstructorArguments>
struct ConstructorWrapper {
  static_assert(std::is_constructible_v<T, ConstructorArguments...>);
};
template <typename T, typename... ArgumentTypes>
struct FunctionWrapper<ConstructorWrapper<T, ArgumentTypes...>> {
  using FunctionPointerType = T (*)(ArgumentTypes...);
  using ArgumentTuple = std::tuple<ArgumentTypes...>;

  static std::vector<Function::ValueDeclaration> GetInputDeclarations(std::span<std::string_view> input_names) {
    std::vector<Function::ValueDeclaration> input_declarations;
    VariableDeclarationsHelper<ArgumentTuple>::Insert(&input_declarations, input_names);
    assert(input_declarations.size() == input_names.size());
    return input_declarations;
  }

  static std::vector<Function::ValueDeclaration> GetOutputDeclarations(std::span<std::string_view> output_names) {
    std::vector<Function::ValueDeclaration> output_declarations;
    VariableDeclarationsHelper<T>::Insert(&output_declarations, output_names);
    assert(output_declarations.size() == output_names.size());
    return output_declarations;
  }

  template <ConstructorWrapper<T, ArgumentTypes...> CONSTRUCTOR>
  static void Call(ExecutionContext* context) {
    context->PushValue(std::make_from_tuple<T>(context->GetValue<ArgumentTuple>(0)));
  }
};

}  // namespace detail

template <typename T, typename... ConstructorArguments>
inline constexpr detail::ConstructorWrapper<T, ConstructorArguments...> Constructor{};


template <auto FUNCTION>
inline safe_ptr<Function> Module::RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                                   std::vector<std::string_view> output_names) {
  return RegisterFunction(
      name,
      &detail::FunctionWrapper<decltype(FUNCTION)>::template Call<FUNCTION>,
      detail::FunctionWrapper<decltype(FUNCTION)>::GetInputDeclarations(input_names), 
      detail::FunctionWrapper<decltype(FUNCTION)>::GetOutputDeclarations(output_names));
}

inline safe_ptr<Function> Module::GetFunction(std::string_view name) {
  for (auto& function : functions_) {
    if (function.name_ == name) {
      return safe_ptr(&function);
    }
  }
  return nullptr;
}

}  // namespace vm
}  // namespace ovis
