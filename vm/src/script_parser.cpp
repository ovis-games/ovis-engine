#include "ovis/vm/script_parser.hpp"

namespace ovis {

Result<ParseScriptResult, ParseScriptErrors> ParseScript(VirtualMachine* virtual_machine, const json& script) {
  assert(script.is_array());
  ParseScriptResult result;
  ParseScriptErrors errors;

  for (const auto& definition : script.items()) {
    assert(definition.value().is_object());
    assert(definition.value().contains("definitionType"));
    const std::string& definition_type = definition.value().at("definitionType");
    if (definition_type == "function") {
      auto function = ParseScriptFunction(virtual_machine, definition.value());
      if (!function) {
        errors.insert(errors.end(), function.error().begin(), function.error().end());
      } else {
        result.functions.push_back(std::move(*function));
      }
    } else if (definition_type == "type") {
      auto type = ParseScriptType(virtual_machine, definition.value());
      if (!type) {
        errors.insert(errors.end(), type.error().begin(), type.error().end());
      } else {
        result.types.push_back(std::move(*type));
      }
    } else {
      errors.emplace_back(ScriptErrorLocation("", "/{}", definition.key()), "Invalid definition type {}",
                          definition_type);
    }
  }

  if (errors.size() > 0) {
    return errors;
  } else {
    return result;
  }
}

ScriptParser::ScriptParser(NotNull<VirtualMachine*> virtual_machine, std::string_view module_name)
    : virtual_machine_(virtual_machine), module_(virtual_machine->GetModule(module_name)) {
  if (module_ == nullptr) {
    module_ = virtual_machine_->RegisterModule(module_name);
    assert(module_);
  }
}

void ScriptParser::AddScript(json script_definition, std::string_view name) {
  assert(script_definition.is_array());

  for (auto& definition : script_definition.items()) {
    assert(definition.value().is_object());
    assert(definition.value().contains("definitionType"));
    const std::string& definition_type = definition.value().at("definitionType");
    const std::string& name = definition.value().at("name");
    if (definition_type == "function") {
      function_definitions_.insert(std::make_pair(name, FunctionDefinition{
            .definition = std::move(definition),
            .script_name = std::string(name),
            .function = nullptr
      }));
    } else if (definition_type == "type") {
      type_definitions_.insert(std::make_pair(name, TypeDefinition{
            .definition = std::move(definition),
            .script_name = std::string(name),
            .type_id = Type::NONE_ID,
      }));
    } else {
      errors_.emplace_back(ScriptErrorLocation(name, "/{}", definition.key()), "Invalid definition type {}",
                           definition_type);
    }
  }
}

}  // namespace ovis
