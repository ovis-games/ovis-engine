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
      errors.emplace_back(fmt::format("/{}", definition.key()), "Invalid definition type {}", definition_type);
    }
  }

  if (errors.size() > 0) {
    return errors;
  } else {
    return result;
  }
}

}  // namespace ovis
