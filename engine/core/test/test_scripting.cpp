#define private public
#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/script_instruction.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/core/script_execution_context.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_parser.hpp>

using namespace ovis;

TEST_CASE("Script Function Parsing", "[ovis][core][script]") {
  LoadCoreModule();

  ScriptFunctionParser parser(json::parse(R"(
    {
      "inputs": {
      },
      "outputs": {
      },
      "actions": [
        {
          "type": "function_call",
          "function": {
            "name": "Add",
            "module": "Core"
          },
          "inputs": {
            "x": {
              "inputType": "constant",
              "type": "Number",
              "value": 10
            },
            "y": 20
          },
          "outputs": {
            "result": "Awesome Variable"
          }
        }
      ]
    }
  )"));

  for (const auto& error : parser.errors) {
    std::cout << error.message;
  }
  REQUIRE(parser.errors.size() == 0);
  REQUIRE(parser.inputs.size() == 0);
  REQUIRE(parser.outputs.size() == 0);

  const ScriptFunction::DebugInfo& debug_info = parser.debug_info;
  REQUIRE(debug_info.scope_info.size() == 1);
  REQUIRE(debug_info.scope_info[0].variables.size() == 1);
  REQUIRE(debug_info.scope_info[0].variables[0].declaration.type == Type::Get<double>());
  REQUIRE(debug_info.scope_info[0].variables[0].declaration.name == "Awesome Variable");
  REQUIRE(debug_info.scope_info[0].variables[0].position == 0);
}

TEST_CASE("Registering script function", "[ovis][core][script]") {
}

