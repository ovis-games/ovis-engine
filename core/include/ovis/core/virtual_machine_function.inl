// namespace ovis {
// namespace vm {

// inline std::shared_ptr<Function> Function::Deserialize(const json& data) {
//   if (!data.contains("module")) {
//     return nullptr;
//   }
//   const auto& module_json = data.at("module");
//   if (!module_json.is_string()) {
//     return nullptr;
//   }
//   const std::shared_ptr<vm::Module> module = Module::Get(module_json.get_ref<const std::string&>());
//   if (module == nullptr) {
//     return nullptr;
//   }
//  if (!data.contains("name")) {
//     return nullptr;
//   }
//   const auto& name_json = data.at("name");
//   if (!name_json.is_string()) {
//     return nullptr;
//   }
//   return module->GetFunction(name_json.get_ref<const std::string&>());
// }

// template <typename... OutputTypes, typename... InputTypes>
// inline FunctionResultType<OutputTypes...> Function::Call(InputTypes&&... inputs) {
//   return Call<OutputTypes...>(ExecutionContext::global_context(), std::forward<InputTypes>(inputs)...);
// }

// template <typename... OutputTypes, typename... InputTypes>
// inline FunctionResultType<OutputTypes...> Function::Call(ExecutionContext* context, InputTypes&&... inputs) {
//   assert(sizeof...(InputTypes) == inputs_.size());
//   // TODO: validate input/output types
//   context->PushStackFrame();
//   ((context->PushValue2(Value::CreateViewIfPossible(std::forward<InputTypes>(inputs)))), ...);
//   function_pointer_(context);
//   if constexpr (sizeof...(OutputTypes) == 0) {
//     context->PopStackFrame();
//   } else {
//     auto result = context->GetTopValue<FunctionResultType<OutputTypes...>>();
//     context->PopStackFrame();
//     return result;
//   }
// }

// inline std::optional<std::size_t> Function::GetInputIndex(std::string_view input_name) const {
//   for (const auto& input : IndexRange(inputs_)) {
//     if (input->name == input_name) {
//       return input.index();
//     }
//   }
//   return {};
// }

// inline std::optional<Function::ValueDeclaration> Function::GetInput(std::string_view input_name) const {
//   for (const auto& input : inputs_) {
//     if (input.name == input_name) {
//       return input;
//     }
//   }
//   return {};
// }

// inline std::optional<Function::ValueDeclaration> Function::GetInput(std::size_t input_index) const {
//   if (input_index < inputs_.size()) {
//     return inputs_[input_index];
//   } else {
//     return {};
//   }
// }

// inline std::optional<std::size_t> Function::GetOutputIndex(std::string_view output_name) const {
//   for (const auto& output : IndexRange(outputs_)) {
//     if (output->name == output_name) {
//       return output.index();
//     }
//   }
//   return {};
// }

// inline std::optional<Function::ValueDeclaration> Function::GetOutput(std::string_view output_name) const {
//   for (const auto& output : outputs_) {
//     if (output.name == output_name) {
//       return output;
//     }
//   }
//   return {};
// }

// inline std::optional<Function::ValueDeclaration> Function::GetOutput(std::size_t output_index) const {
//   if (output_index < outputs_.size()) {
//     return outputs_[output_index];
//   } else {
//     return {};
//   }
// }

// template <typename... InputTypes>
// inline bool Function::IsCallableWithArguments() const {
//   if (inputs_.size() != sizeof...(InputTypes)) {
//     return false;
//   }
//   std::array<std::shared_ptr<Type>, sizeof...(InputTypes)> input_types = {
//     Type::Get<InputTypes>()...
//   };

//   for (auto i : IRange(sizeof...(InputTypes))) {
//     if (inputs_[i].type.lock() != input_types[i]) {
//       return false;
//     }
//   }
//   return true;
// }

// inline Function::Function(std::string_view name, FunctionPointer function_pointer, std::vector<ValueDeclaration> inputs,
//                           std::vector<ValueDeclaration> outputs)
//     : name_(name), function_pointer_(function_pointer), inputs_(inputs), outputs_(outputs) {
//   text_ = name_;
//   for (const auto input: inputs_) {
//     text_ += fmt::format(" ({})", input.name);
//   }
// }

// inline json Function::Serialize() const {
//   json function = json::object();
//   function["name"] = name();
//   json& inputs = function["inputs"] = json::array();
//   for (const auto& input : this->inputs()) {
//     const auto input_type = input.type.lock();
//     inputs.push_back({
//       { "name", input.name },
//       { "type", input_type ? json(std::string(input_type->full_reference())) : json() },
//     });
//   }
//   json& outputs = function["outputs"] = json::array();
//   for (const auto& output : this->outputs()) {
//     const auto output_type = output.type.lock();
//     outputs.push_back({
//       { "name", output.name },
//       { "type", output_type ? json(std::string(output_type->full_reference())) : json() },
//     });
//   }
//   return function;
// }

// }
// }
