#include <ovis/vm/script_type_parser.hpp>

namespace ovis {

Result<ParseScriptTypeResult, ParseScriptErrors> ParseScriptType(VirtualMachine* virtual_machine,
                                                                 const json& type_definition) {
  TypeDescription description = {
    .memory_layout = {
      .alignment_in_bytes = ValueStorage::ALIGNMENT,
      .size_in_bytes = 0,
    }
  };
  ParseScriptErrors errors;
  std::vector<TypePropertyDescription> properties;
  for (const auto& [property_name, property_definition] : type_definition["properties"].items()) {
    const auto& property_type = virtual_machine->GetType(property_definition.at("type"));
    if (!property_type) {
      errors.emplace_back(
          fmt::format("Invalid type for property {}: {}", property_name, property_definition.at("type")),
          fmt::format("/properties/{}", property_name));
      continue;
    }
    if (property_type->alignment_in_bytes() > description.memory_layout.alignment_in_bytes) {
      description.memory_layout.alignment_in_bytes = property_type->alignment_in_bytes();
    }
    const std::size_t padding_bytes = description.memory_layout.size_in_bytes % property_type->alignment_in_bytes();
    description.memory_layout.size_in_bytes += padding_bytes;

    description.properties.push_back({
        .name = property_name,
        .type = property_type->id(),
        .access = TypePropertyDescription::PrimitiveAccess { .offset = description.memory_layout.size_in_bytes }
    });

    description.memory_layout.size_in_bytes += property_type->size_in_bytes();
  }

  return ParseScriptTypeResult{ description };
}

}  // namespace ovis
