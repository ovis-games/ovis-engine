#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/script_instruction.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_parser.hpp>
#include <ovis/core/script_function.hpp>

using namespace ovis;
using namespace ovis::vm;

const ovis::json add_10_and_20 = ovis::json::parse(R"(
    {
      "inputs": [],
      "outputs": [
        {
          "name": "result",
          "type": "Number",
          "module": "Core"
        }
      ],
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
        },
        {
          "type": "return",
          "outputs": {
            "result": {
              "inputType": "local_variable",
              "name": "Awesome Variable"
            }
          }
        }
      ]
    }
  )");

TEST_CASE("Simple return", "[ovis][core][script]") {
  const ovis::json return_10 = ovis::json::parse(R"(
    {
      "inputs": [],
      "outputs": [
        {
          "name": "result",
          "type": "Number",
          "module": "Core"
        }
      ],
      "actions": [
        {
          "type": "return",
          "outputs": {
            "result": 10
          }
        }
      ]
    }
  )");
  LoadCoreModule();

  ScriptFunctionParser parser(return_10);
  REQUIRE(parser.errors.size() == 0);
  REQUIRE(parser.inputs.size() == 0);
  REQUIRE(parser.outputs.size() == 1);
  REQUIRE(parser.outputs[0].name == "result");
  REQUIRE(parser.outputs[0].type == Type::Get<double>());

  REQUIRE(parser.instructions.size() == 2);
  REQUIRE(std::holds_alternative<instructions::PushConstant>(parser.instructions[0]));
  REQUIRE(std::holds_alternative<instructions::Return>(parser.instructions[1]));

  ScriptFunction f(parser);
  const double result = f.Call<double>();
  REQUIRE(result == 10);
}

TEST_CASE("Script Function Parsing", "[ovis][core][script]") {
  LoadCoreModule();

  ScriptFunctionParser parser(add_10_and_20);

  for (const auto& error : parser.errors) {
    std::cout << error.message;
  }
  REQUIRE(parser.errors.size() == 0);
  REQUIRE(parser.inputs.size() == 0);
  REQUIRE(parser.outputs.size() == 1);
  REQUIRE(parser.outputs[0].name == "result");
  REQUIRE(parser.outputs[0].type == Type::Get<double>());

  const ScriptFunction::DebugInfo& debug_info = parser.debug_info;
  REQUIRE(debug_info.scope_info.size() == 1);
  REQUIRE(debug_info.scope_info[0].variables.size() == 1);
  REQUIRE(debug_info.scope_info[0].variables[0].declaration.type == Type::Get<double>());
  REQUIRE(debug_info.scope_info[0].variables[0].declaration.name == "Awesome Variable");
  REQUIRE(debug_info.scope_info[0].variables[0].position == 0);

  // REQUIRE(parser.instructions.size() == 9);
  REQUIRE(std::holds_alternative<instructions::PushConstant>(parser.instructions[0]));
  REQUIRE(std::holds_alternative<instructions::PushStackFrame>(parser.instructions[1]));
  REQUIRE(std::holds_alternative<instructions::PushConstant>(parser.instructions[2]));
  REQUIRE(std::holds_alternative<instructions::PushConstant>(parser.instructions[3]));
  REQUIRE(std::holds_alternative<instructions::NativeFunctionCall>(parser.instructions[4]));
  REQUIRE(std::holds_alternative<instructions::Assign>(parser.instructions[5]));
  REQUIRE(std::holds_alternative<instructions::PopStackFrame>(parser.instructions[6]));
  REQUIRE(std::holds_alternative<instructions::PushStackValue>(parser.instructions[7]));
  REQUIRE(std::holds_alternative<instructions::Return>(parser.instructions[8]));

  ScriptFunction function(parser);
  REQUIRE(function.inputs().size() == 0);
  REQUIRE(function.outputs().size() == 1);
  REQUIRE(function.outputs()[0].name == "result");
  REQUIRE(function.outputs()[0].type == Type::Get<double>());
  const double result = function.Call<double>();
  REQUIRE(result == 30);
}

TEST_CASE("Factorial", "[ovis][core][script]") {
  const ovis::json factorial_json = ovis::json::parse(R"(
    {
      "inputs": [
        {
          "name": "n",
          "type": "Number",
          "module": "Core"
        }
      ],
      "outputs": [
        {
          "name": "result",
          "type": "Number",
          "module": "Core"
        }
      ],
      "actions": [
        {
          "type": "function_call",
          "function": {
            "name": "Create Number",
            "module": "Core"
          },
          "inputs": {
            "value": 1
          },
          "outputs": {
            "result": "result"
          }
        },
        {
          "type": "function_call",
          "function": {
            "name": "Is greater",
            "module": "Core"
          },
          "inputs": {
            "x": {
              "inputType": "local_variable",
              "name": "n"
            },
            "y": 1
          },
          "outputs": {
            "result": "continue"
          }
        },
        {
          "type": "while",
          "condition": {
            "inputType": "local_variable",
            "name": "continue"
          },
          "actions": [
            {
              "type": "function_call",
              "function": {
                "name": "Multiply",
                "module": "Core"
              },
              "inputs": {
                "x": {
                  "inputType": "local_variable",
                  "name": "n"
                },
                "y": {
                  "inputType": "local_variable",
                  "name": "result"
                }
              },
              "outputs": {
                "result": "result"
              }
            },
            {
              "type": "function_call",
              "function": {
                "name": "Subtract",
                "module": "Core"
              },
              "inputs": {
                "x": {
                  "inputType": "local_variable",
                  "name": "n"
                },
                "y": 1
              },
              "outputs": {
                "result": "n"
              }
            },
            {
              "type": "function_call",
              "function": {
                "name": "Is greater",
                "module": "Core"
              },
              "inputs": {
                "x": {
                  "inputType": "local_variable",
                  "name": "n"
                },
                "y": 1
              },
              "outputs": {
                "result": "continue"
              }
            }
          ]
        },
        {
          "type": "return",
          "outputs": {
            "result": {
              "inputType": "local_variable",
              "name": "result"
            }
          }
        }
      ]
    }
  )");

  ScriptFunctionParser parser(factorial_json);
  for (const auto& error : parser.errors) {
    std::cout << error.message << std::endl;
  }
  REQUIRE(parser.errors.size() == 0);
  

  ovis::ScriptFunction factorial(factorial_json);
  REQUIRE(factorial.inputs().size() == 1);
  REQUIRE(factorial.outputs().size() == 1);

  REQUIRE(factorial.Call<double>(1.0) == 1);
  REQUIRE(factorial.Call<double>(2.0) == 2);
  REQUIRE(factorial.Call<double>(3.0) == 6);
  REQUIRE(factorial.Call<double>(4.0) == 24);
  REQUIRE(factorial.Call<double>(5.0) == 120);
  REQUIRE(factorial.Call<double>(18.0) == 6402373705728000); // Max factorial that's still a safe integer
}

