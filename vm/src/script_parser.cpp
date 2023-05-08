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
      errors.insert(errors.end(), function.errors.begin(), function.errors.end());
      result.functions.push_back(std::move(function.function_description));
    } else if (definition_type == "type") {
      auto type = ParseScriptType(virtual_machine, definition.value());
      if (!type) {
        errors.insert(errors.end(), type.error().begin(), type.error().end());
      } else {
        result.types.push_back(std::move(*type));
      }
    } else {
      errors.emplace_back(ScriptErrorLocation("", fmt::format("/{}", definition.key())), "Invalid definition type {}",
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
    : virtual_machine_(virtual_machine), module_(module_name) {
  if (!virtual_machine->IsModuleRegistered(module_name)) {
    virtual_machine_->RegisterModule(module_name);
  }
}

void ScriptParser::AddScript(json script_definition, std::string_view script_name) {
  assert(script_definition.is_array());

  for (auto& definition : script_definition.items()) {
    assert(definition.value().is_object());
    assert(definition.value().contains("definitionType"));
    const std::string& definition_type = definition.value().at("definitionType");
    const std::string& name = definition.value().at("name");
    if (definition_type == "function") {
      function_definitions_.insert(std::make_pair(name, FunctionDefinition{
            .definition = std::move(definition.value()),
            .script_name = std::string(script_name),
            .path = fmt::format("/{}", definition.key()),
            .function = nullptr
      }));
    } else if (definition_type == "type") {
      type_definitions_.insert(std::make_pair(name, TypeDefinition{
            .definition = std::move(definition.value()),
            .script_name = std::string(script_name),
            .path = fmt::format("/{}", definition.key()),
            .type_id = Type::NONE_ID,
      }));
    } else {
      errors_.emplace_back(ScriptErrorLocation(name, fmt::format("/{}", definition.key())),
                           "Invalid definition type {}", definition_type);
    }
  }
}

bool ScriptParser::Parse() {
  errors_.clear();
  
  for (auto& type_definition : Values(type_definitions_)) {
    if (type_definition.type_id == Type::NONE_ID) {
      auto parse_type_result = ParseScriptType(virtual_machine_, type_definition.definition,
                                               type_definition.script_name, type_definition.path);
      if (parse_type_result) {
        parse_type_result->type_description.module = module_;
        virtual_machine_->RegisterType(parse_type_result->type_description);
      } else {
        errors_.insert(errors_.end(), parse_type_result.error().begin(), parse_type_result.error().end());
      }
    }
  }

  for (auto& function_definition : Values(function_definitions_)) {
    if (function_definition.function == nullptr) {
      auto parse_function_result = ParseScriptFunction(virtual_machine_, function_definition.definition,
                                                       function_definition.script_name, function_definition.path);
      if (parse_function_result.errors.empty()) {
        parse_function_result.function_description.module = module_;
        virtual_machine_->RegisterFunction(parse_function_result.function_description);
      } else {
        errors_.insert(errors_.end(), parse_function_result.errors.begin(), parse_function_result.errors.end());
      }
    }
  }

  return errors_.size() == 0;
}

}  // namespace ovis
